import math
import struct
import json
from typing import List, Optional, Tuple


D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT = 0x10


D3D11_TEXTURE_ADDRESS_WRAP = 1
D3D11_TEXTURE_ADDRESS_MIRROR = 2
D3D11_TEXTURE_ADDRESS_CLAMP = 3
D3D11_TEXTURE_ADDRESS_BORDER = 4
D3D11_TEXTURE_ADDRESS_MIRROR_ONCE = 5


D3D11_COMPARISON_NEVER = 0
D3D11_COMPARISON_LESS = 1
D3D11_COMPARISON_EQUAL = 2
D3D11_COMPARISON_LESS_EQUAL = 3
D3D11_COMPARISON_GREATER = 4
D3D11_COMPARISON_NOT_EQUAL = 5
D3D11_COMPARISON_GREATER_EQUAL = 6
D3D11_COMPARISON_ALWAYS = 7


class Sampler:
    def __init__(
        self,
        Filter: int = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
        AddressU: int = D3D11_TEXTURE_ADDRESS_WRAP,
        AddressV: int = D3D11_TEXTURE_ADDRESS_WRAP,
        AddressW: int = D3D11_TEXTURE_ADDRESS_WRAP,
        MipLODBias: float = 0.0,
        MaxAnisotropy: int = 1,
        ComparisonFunc: int = D3D11_COMPARISON_NEVER,
        BorderColor: Optional[List[float]] = None,
        MinLOD: float = -3.40282e38,
        MaxLOD: float = 3.40282e38
    ):
        self.Filter = Filter
        self.AddressU = AddressU
        self.AddressV = AddressV
        self.AddressW = AddressW
        self.MipLODBias = MipLODBias
        self.MaxAnisotropy = MaxAnisotropy
        self.ComparisonFunc = ComparisonFunc
        self.BorderColor = BorderColor if BorderColor is not None else [0.0, 0.0, 0.0, 0.0]
        self.MinLOD = MinLOD
        self.MaxLOD = MaxLOD

    @classmethod
    def from_json(cls, json_path: str) -> 'Sampler':
        with open(json_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        return cls(
            Filter=data.get('Filter', D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT),
            AddressU=data.get('AddressU', D3D11_TEXTURE_ADDRESS_WRAP),
            AddressV=data.get('AddressV', D3D11_TEXTURE_ADDRESS_WRAP),
            AddressW=data.get('AddressW', D3D11_TEXTURE_ADDRESS_WRAP),
            MipLODBias=data.get('MipLODBias', 0.0),
            MaxAnisotropy=data.get('MaxAnisotropy', 1),
            ComparisonFunc=data.get('ComparisonFunc', D3D11_COMPARISON_NEVER),
            BorderColor=data.get('BorderColor', [0.0, 0.0, 0.0, 0.0]),
            MinLOD=data.get('MinLOD', -3.40282e38),
            MaxLOD=data.get('MaxLOD', 3.40282e38)
        )

    def _get_filter_mode(self) -> Tuple[int, int, int]:
        min_filter = (self.Filter >> 0) & 0x03
        mag_filter = (self.Filter >> 4) & 0x03
        mip_filter = (self.Filter >> 8) & 0x03
        return min_filter, mag_filter, mip_filter

    def _address_mode_to_func(self, address_mode: int):
        if address_mode == D3D11_TEXTURE_ADDRESS_WRAP:
            return self._wrap_address
        elif address_mode == D3D11_TEXTURE_ADDRESS_MIRROR:
            return self._mirror_address
        elif address_mode == D3D11_TEXTURE_ADDRESS_CLAMP:
            return self._clamp_address
        elif address_mode == D3D11_TEXTURE_ADDRESS_BORDER:
            return self._border_address
        elif address_mode == D3D11_TEXTURE_ADDRESS_MIRROR_ONCE:
            return self._mirror_once_address
        return self._wrap_address

    def _wrap_address(self, coord: float) -> float:
        if coord < 0.0:
            coord = coord - math.floor(coord)
        elif coord >= 1.0:
            coord = coord - math.floor(coord)
        return coord

    def _mirror_address(self, coord: float) -> float:
        if coord < 0.0:
            coord = -coord
        int_part = int(coord)
        frac = coord - int_part
        if int_part % 2 == 0:
            return frac
        else:
            return 1.0 - frac

    def _clamp_address(self, coord: float) -> float:
        return max(0.0, min(1.0, coord))

    def _border_address(self, coord: float) -> float:
        return coord

    def _mirror_once_address(self, coord: float) -> float:
        if coord < 0.0:
            return -coord
        elif coord > 1.0:
            return 2.0 - coord
        return coord

    def transform_coordinates(self, u: float, v: float, w: float) -> Tuple[float, float, float]:
        address_u_func = self._address_mode_to_func(self.AddressU)
        address_v_func = self._address_mode_to_func(self.AddressV)
        address_w_func = self._address_mode_to_func(self.AddressW)
        return address_u_func(u), address_v_func(v), address_w_func(w)


class Texture:
    def __init__(self, file_path: str, sampler: Sampler):
        self.file_path = file_path
        self.sampler = sampler
        self.width = 0
        self.height = 0
        self.mip_levels: List[List[List[List[float]]]] = []
        self._load_texture()

    def _load_texture(self):
        try:
            with open(self.file_path, 'rb') as f:
                data = f.read()
            self._parse_bmp(data)
        except Exception:
            self._create_placeholder_texture()

    def _parse_bmp(self, data: bytes):
        if len(data) < 54:
            self._create_placeholder_texture()
            return

        if data[0:2] != b'BM':
            self._create_placeholder_texture()
            return

        offset = struct.unpack('<I', data[10:14])[0]
        header_size = struct.unpack('<I', data[14:18])[0]

        if header_size == 40:
            self.width = struct.unpack('<I', data[18:22])[0]
            self.height = struct.unpack('<I', data[22:26])[0]
            bits_per_pixel = struct.unpack('<H', data[28:30])[0]

            if bits_per_pixel != 24 and bits_per_pixel != 32:
                self._create_placeholder_texture()
                return

            bytes_per_pixel = bits_per_pixel // 8
            row_size = ((bits_per_pixel * self.width + 31) // 32) * 4

            pixels = []
            for y in range(self.height):
                row = []
                for x in range(self.width):
                    pos = offset + y * row_size + x * bytes_per_pixel
                    if pos + bytes_per_pixel > len(data):
                        row.append([0.0, 0.0, 0.0, 1.0])
                        continue

                    if bits_per_pixel == 24:
                        b, g, r = data[pos], data[pos + 1], data[pos + 2]
                    else:
                        b, g, r, a = data[pos], data[pos + 1], data[pos + 2], data[pos + 3]

                    row.append([
                        r / 255.0,
                        g / 255.0,
                        b / 255.0,
                        a / 255.0 if bits_per_pixel == 32 else 1.0
                    ])
                pixels.append(row)

            self.mip_levels = [pixels]
            self._generate_mipmaps()

        else:
            self._create_placeholder_texture()

    def _create_placeholder_texture(self):
        self.width = 4
        self.height = 4
        checkerboard = []
        for y in range(self.height):
            row = []
            for x in range(self.width):
                if (x + y) % 2 == 0:
                    color = [1.0, 1.0, 1.0, 1.0]
                else:
                    color = [0.0, 0.0, 0.0, 1.0]
                row.append(color)
            checkerboard.append(row)
        self.mip_levels = [checkerboard]
        self._generate_mipmaps()

    def _generate_mipmaps(self):
        while self.mip_levels[-1]:
            prev_level = self.mip_levels[-1]
            h = len(prev_level)
            w = len(prev_level[0])

            if w <= 1 or h <= 1:
                break

            new_h = max(1, h // 2)
            new_w = max(1, w // 2)
            new_level = []

            for y in range(new_h):
                new_row = []
                for x in range(new_w):
                    c00 = prev_level[y * 2][x * 2]
                    c10 = prev_level[min(y * 2 + 1, h - 1)][x * 2]
                    c01 = prev_level[y * 2][min(x * 2 + 1, w - 1)]
                    c11 = prev_level[min(y * 2 + 1, h - 1)][min(x * 2 + 1, w - 1)]

                    new_color = [
                        (c00[0] + c10[0] + c01[0] + c11[0]) / 4.0,
                        (c00[1] + c10[1] + c01[1] + c11[1]) / 4.0,
                        (c00[2] + c10[2] + c01[2] + c11[2]) / 4.0,
                        (c00[3] + c10[3] + c01[3] + c11[3]) / 4.0
                    ]
                    new_row.append(new_color)
                new_level.append(new_row)

            self.mip_levels.append(new_level)

    def _sample_nearest(self, mip_level: List[List[List[float]]], u: float, v: float) -> List[float]:
        h = len(mip_level)
        w = len(mip_level[0])

        x = int(u * w) % w
        y = int(v * h) % h

        x = max(0, min(w - 1, x))
        y = max(0, min(h - 1, y))

        return mip_level[y][x]

    def _sample_linear(self, mip_level: List[List[List[float]]], u: float, v: float) -> List[float]:
        h = len(mip_level)
        w = len(mip_level[0])

        fu = u * w
        fv = v * h

        u0 = int(fu)
        v0 = int(fv)

        u1 = (u0 + 1) % w
        v1 = (v0 + 1) % h

        s = fu - u0
        t = fv - v0

        u0 = max(0, min(w - 1, u0))
        v0 = max(0, min(h - 1, v0))
        u1 = max(0, min(w - 1, u1))
        v1 = max(0, min(h - 1, v1))

        c00 = mip_level[v0][u0]
        c10 = mip_level[v0][u1]
        c01 = mip_level[v1][u0]
        c11 = mip_level[v1][u1]

        color = [
            c00[0] * (1 - s) * (1 - t) + c10[0] * s * (1 - t) + c01[0] * (1 - s) * t + c11[0] * s * t,
            c00[1] * (1 - s) * (1 - t) + c10[1] * s * (1 - t) + c01[1] * (1 - s) * t + c11[1] * s * t,
            c00[2] * (1 - s) * (1 - t) + c10[2] * s * (1 - t) + c01[2] * (1 - s) * t + c11[2] * s * t,
            c00[3] * (1 - s) * (1 - t) + c10[3] * s * (1 - t) + c01[3] * (1 - s) * t + c11[3] * s * t
        ]
        return color

    def _sample_mip_point(self, mip_level: List[List[List[float]]], u: float, v: float) -> List[float]:
        return self._sample_nearest(mip_level, u, v)

    def sample(self, u: float, v: float, w: float = 0.0) -> List[float]:
        tu, tv, tw = self.sampler.transform_coordinates(u, v, w)

        lod = tw + self.sampler.MipLODBias

        lod = max(self.sampler.MinLOD, min(self.sampler.MaxLOD, lod))

        min_filter, mag_filter, mip_filter = self.sampler._get_filter_mode()

        level_count = len(self.mip_levels)

        if level_count == 1:
            if mag_filter == 0:
                return self._sample_nearest(self.mip_levels[0], tu, tv)
            else:
                return self._sample_linear(self.mip_levels[0], tu, tv)

        lod_level = min(lod, float(level_count - 1))
        level0 = int(lod_level)
        level1 = min(level0 + 1, level_count - 1)

        s = lod_level - level0

        if min_filter == 0 and mag_filter == 0 and mip_filter == 0:
            color0 = self._sample_nearest(self.mip_levels[level0], tu, tv)
            color1 = self._sample_nearest(self.mip_levels[level1], tu, tv)
        elif min_filter == 0 and mag_filter == 0 and mip_filter == 1:
            color0 = self._sample_nearest(self.mip_levels[level0], tu, tv)
            color1 = self._sample_nearest(self.mip_levels[level1], tu, tv)
        elif min_filter == 0 and mag_filter == 0 and mip_filter == 2:
            color0 = self._sample_linear(self.mip_levels[level0], tu, tv)
            color1 = self._sample_linear(self.mip_levels[level1], tu, tv)
        elif min_filter == 0 and mag_filter == 1 and mip_filter == 0:
            color0 = self._sample_linear(self.mip_levels[level0], tu, tv)
            color1 = self._sample_linear(self.mip_levels[level1], tu, tv)
        elif min_filter == 0 and mag_filter == 1 and mip_filter == 1:
            color0 = self._sample_linear(self.mip_levels[level0], tu, tv)
            color1 = self._sample_linear(self.mip_levels[level1], tu, tv)
        elif min_filter == 0 and mag_filter == 1 and mip_filter == 2:
            color0 = self._sample_linear(self.mip_levels[level0], tu, tv)
            color1 = self._sample_linear(self.mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 0 and mip_filter == 0:
            color0 = self._sample_nearest(self.mip_levels[level0], tu, tv)
            color1 = self._sample_nearest(self.mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 0 and mip_filter == 1:
            color0 = self._sample_nearest(self.mip_levels[level0], tu, tv)
            color1 = self._sample_nearest(self.mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 0 and mip_filter == 2:
            color0 = self._sample_linear(self.mip_levels[level0], tu, tv)
            color1 = self._sample_linear(self.mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 1 and mip_filter == 0:
            color0 = self._sample_linear(self.mip_levels[level0], tu, tv)
            color1 = self._sample_linear(self.mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 1 and mip_filter == 1:
            color0 = self._sample_linear(self.mip_levels[level0], tu, tv)
            color1 = self._sample_linear(self.mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 1 and mip_filter == 2:
            color0 = self._sample_linear(self.mip_levels[level0], tu, tv)
            color1 = self._sample_linear(self.mip_levels[level1], tu, tv)
        else:
            color0 = self._sample_linear(self.mip_levels[level0], tu, tv)
            color1 = self._sample_linear(self.mip_levels[level1], tu, tv)

        result = [
            color0[0] * (1 - s) + color1[0] * s,
            color0[1] * (1 - s) + color1[1] * s,
            color0[2] * (1 - s) + color1[2] * s,
            color0[3] * (1 - s) + color1[3] * s
        ]

        return [max(0.0, min(1.0, c)) for c in result]