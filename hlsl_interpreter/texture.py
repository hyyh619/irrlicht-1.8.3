import math
import struct
import json
import os
from typing import List, Optional, Tuple, Dict


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

FILTER_MAP = {
    "D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT": 0x10,
    "D3D11_FILTER_MIN_MAG_MIP_LINEAR": 0x15,
    "D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR": 0x14,
    "D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT": 0x11,
}

ADDRESS_MAP = {
    "D3D11_TEXTURE_ADDRESS_WRAP": 1,
    "D3D11_TEXTURE_ADDRESS_MIRROR": 2,
    "D3D11_TEXTURE_ADDRESS_CLAMP": 3,
    "D3D11_TEXTURE_ADDRESS_BORDER": 4,
    "D3D11_TEXTURE_ADDRESS_MIRROR_ONCE": 5,
}

COMPARISON_MAP = {
    "D3D11_COMPARISON_NEVER": 0,
    "D3D11_COMPARISON_LESS": 1,
    "D3D11_COMPARISON_EQUAL": 2,
    "D3D11_COMPARISON_LESS_EQUAL": 3,
    "D3D11_COMPARISON_GREATER": 4,
    "D3D11_COMPARISON_NOT_EQUAL": 5,
    "D3D11_COMPARISON_GREATER_EQUAL": 6,
    "D3D11_COMPARISON_ALWAYS": 7,
}

FORMAT_MAP = {
    "DXGI_FORMAT_B8G8R8A8_UNORM": 0x57,
    "DXGI_FORMAT_R8G8B8A8_UNORM": 0x56,
    "DXGI_FORMAT_R8G8B8A8_UINT": 0x6C,
    "DXGI_FORMAT_R32G32B32A32_FLOAT": 0x5A,
    "DXGI_FORMAT_R32G32B32A32_UINT": 0x5B,
    "DXGI_FORMAT_R16G16B16A16_FLOAT": 0x5E,
    "DXGI_FORMAT_R16G16B16A16_UNORM": 0x5C,
    "DXGI_FORMAT_R32_FLOAT": 0x52,
    "DXGI_FORMAT_R8_UNORM": 0x51,
}

USAGE_MAP = {
    "D3D11_USAGE_DEFAULT": 0,
    "D3D11_USAGE_IMMUTABLE": 1,
    "D3D11_USAGE_DYNAMIC": 2,
    "D3D11_USAGE_STAGING": 3,
}

BINDFLAGS_MAP = {
    "D3D11_BIND_SHADER_RESOURCE": 0x8,
    "D3D11_BIND_RENDER_TARGET": 0x4,
    "D3D11_BIND_DEPTH_STENCIL": 0x2,
    "D3D11_BIND_VERTEX_BUFFER": 0x1,
    "D3D11_BIND_INDEX_BUFFER": 0x2,
}


def _convert_filter(val):
    if isinstance(val, int):
        return val
    return FILTER_MAP.get(val, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT)


def _convert_address(val):
    if isinstance(val, int):
        return val
    return ADDRESS_MAP.get(val, D3D11_TEXTURE_ADDRESS_WRAP)


def _convert_comparison(val):
    if isinstance(val, int):
        return val
    return COMPARISON_MAP.get(val, D3D11_COMPARISON_NEVER)


def _convert_format(val):
    if isinstance(val, int):
        return val
    return FORMAT_MAP.get(val, 0x57)


def _convert_usage(val):
    if isinstance(val, int):
        return val
    return USAGE_MAP.get(val, 0)


def _convert_bindflags(val):
    if isinstance(val, int):
        return val
    return BINDFLAGS_MAP.get(val, 0x8)


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
    def from_config(cls, config_path: str, sampler_id: int) -> 'Sampler':
        with open(config_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        sampler_data = data[str(sampler_id)]
        return cls(
            Filter=_convert_filter(sampler_data.get('Filter', D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT)),
            AddressU=_convert_address(sampler_data.get('AddressU', D3D11_TEXTURE_ADDRESS_WRAP)),
            AddressV=_convert_address(sampler_data.get('AddressV', D3D11_TEXTURE_ADDRESS_WRAP)),
            AddressW=_convert_address(sampler_data.get('AddressW', D3D11_TEXTURE_ADDRESS_WRAP)),
            MipLODBias=sampler_data.get('MipLODBias', 0.0),
            MaxAnisotropy=sampler_data.get('MaxAnisotropy', 1),
            ComparisonFunc=_convert_comparison(sampler_data.get('ComparisonFunc', D3D11_COMPARISON_NEVER)),
            BorderColor=sampler_data.get('BorderColor', [0.0, 0.0, 0.0, 0.0]),
            MinLOD=sampler_data.get('MinLOD', -3.40282e38),
            MaxLOD=sampler_data.get('MaxLOD', 3.40282e38)
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


class TextureDesc:
    def __init__(
        self,
        Width: int = 512,
        Height: int = 512,
        MipLevels: int = 1,
        ArraySize: int = 1,
        Format: int = 0x57,
        SampleDesc_Count: int = 1,
        SampleDesc_Quality: int = 0,
        Usage: int = 0,
        BindFlags: int = 0x8,
        CPUAccessFlags: int = 0,
        MiscFlags: int = 0,
        DataPath: str = ""
    ):
        self.Width = Width
        self.Height = Height
        self.MipLevels = MipLevels
        self.ArraySize = ArraySize
        self.Format = Format
        self.SampleDesc_Count = SampleDesc_Count
        self.SampleDesc_Quality = SampleDesc_Quality
        self.Usage = Usage
        self.BindFlags = BindFlags
        self.CPUAccessFlags = CPUAccessFlags
        self.MiscFlags = MiscFlags
        self.DataPath = DataPath

    @classmethod
    def from_config(cls, config_path: str, texture_id: int) -> 'TextureDesc':
        with open(config_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        tex_data = data[str(texture_id)]
        return cls(
            Width=tex_data.get('Width', 512),
            Height=tex_data.get('Height', 512),
            MipLevels=tex_data.get('MipLevels', 1),
            ArraySize=tex_data.get('ArraySize', 1),
            Format=_convert_format(tex_data.get('Format', 'DXGI_FORMAT_B8G8R8A8_UNORM')),
            SampleDesc_Count=tex_data.get('SampleDesc', {}).get('Count', 1),
            SampleDesc_Quality=tex_data.get('SampleDesc', {}).get('Quality', 0),
            Usage=_convert_usage(tex_data.get('Usage', 'D3D11_USAGE_DEFAULT')),
            BindFlags=_convert_bindflags(tex_data.get('BindFlags', 'D3D11_BIND_SHADER_RESOURCE')),
            CPUAccessFlags=tex_data.get('CPUAccessFlags', 0),
            MiscFlags=tex_data.get('MiscFlags', 0),
            DataPath=tex_data.get('Image', '')
        )


class Texture:
    def __init__(self):
        self._mip_levels_cache: Dict[str, List[List[List[List[float]]]]] = {}

    def _get_mip_levels(self, texture_desc: TextureDesc) -> List[List[List[List[float]]]]:
        data_path = texture_desc.DataPath
        if data_path in self._mip_levels_cache:
            return self._mip_levels_cache[data_path]

        mip_levels = []
        try:
            with open(data_path, 'rb') as f:
                data = f.read()
            mip_levels = self._parse_bmp(data, texture_desc)
        except Exception:
            mip_levels = self._create_placeholder_texture(texture_desc)

        self._mip_levels_cache[data_path] = mip_levels
        return mip_levels

    def _parse_bmp(self, data: bytes, texture_desc: TextureDesc) -> List[List[List[List[float]]]]:
        mip_levels = []
        if len(data) < 54:
            return self._create_placeholder_texture(texture_desc)

        if data[0:2] != b'BM':
            return self._create_placeholder_texture(texture_desc)

        offset = struct.unpack('<I', data[10:14])[0]
        header_size = struct.unpack('<I', data[14:18])[0]

        if header_size == 40:
            width = struct.unpack('<I', data[18:22])[0]
            height = struct.unpack('<I', data[22:26])[0]
            bits_per_pixel = struct.unpack('<H', data[28:30])[0]

            if bits_per_pixel != 24 and bits_per_pixel != 32:
                return self._create_placeholder_texture(texture_desc)

            bytes_per_pixel = bits_per_pixel // 8
            row_size = ((bits_per_pixel * width + 31) // 32) * 4

            pixels = []
            for y in range(height):
                row = []
                for x in range(width):
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

            mip_levels = [pixels]
            mip_levels.extend(self._generate_mipmaps(pixels))
        else:
            return self._create_placeholder_texture(texture_desc)

        return mip_levels

    def _create_placeholder_texture(self, texture_desc: TextureDesc) -> List[List[List[List[float]]]]:
        width = 4
        height = 4
        checkerboard = []
        for y in range(height):
            row = []
            for x in range(width):
                if (x + y) % 2 == 0:
                    color = [1.0, 1.0, 1.0, 1.0]
                else:
                    color = [0.0, 0.0, 0.0, 1.0]
                row.append(color)
            checkerboard.append(row)
        mip_levels = [checkerboard]
        mip_levels.extend(self._generate_mipmaps(checkerboard))
        return mip_levels

    def _generate_mipmaps(self, base_level: List[List[List[float]]]) -> List[List[List[List[float]]]]:
        mip_levels = []
        prev_level = base_level
        h = len(prev_level)
        w = len(prev_level[0])

        while True:
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

            mip_levels.append(new_level)
            prev_level = new_level
            h = new_h
            w = new_w

        return mip_levels

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

    def sample(self, u: float, v: float, w: float, texture_desc: TextureDesc, sampler: Sampler) -> List[float]:
        tu, tv, tw = sampler.transform_coordinates(u, v, w)

        lod = tw + sampler.MipLODBias

        lod = max(sampler.MinLOD, min(sampler.MaxLOD, lod))

        min_filter, mag_filter, mip_filter = sampler._get_filter_mode()

        mip_levels = self._get_mip_levels(texture_desc)
        level_count = len(mip_levels)

        if level_count == 1:
            if mag_filter == 0:
                return self._sample_nearest(mip_levels[0], tu, tv)
            else:
                return self._sample_linear(mip_levels[0], tu, tv)

        lod_level = min(lod, float(level_count - 1))
        level0 = int(lod_level)
        level1 = min(level0 + 1, level_count - 1)

        s = lod_level - level0

        if min_filter == 0 and mag_filter == 0 and mip_filter == 0:
            color0 = self._sample_nearest(mip_levels[level0], tu, tv)
            color1 = self._sample_nearest(mip_levels[level1], tu, tv)
        elif min_filter == 0 and mag_filter == 0 and mip_filter == 1:
            color0 = self._sample_nearest(mip_levels[level0], tu, tv)
            color1 = self._sample_nearest(mip_levels[level1], tu, tv)
        elif min_filter == 0 and mag_filter == 0 and mip_filter == 2:
            color0 = self._sample_linear(mip_levels[level0], tu, tv)
            color1 = self._sample_linear(mip_levels[level1], tu, tv)
        elif min_filter == 0 and mag_filter == 1 and mip_filter == 0:
            color0 = self._sample_linear(mip_levels[level0], tu, tv)
            color1 = self._sample_linear(mip_levels[level1], tu, tv)
        elif min_filter == 0 and mag_filter == 1 and mip_filter == 1:
            color0 = self._sample_linear(mip_levels[level0], tu, tv)
            color1 = self._sample_linear(mip_levels[level1], tu, tv)
        elif min_filter == 0 and mag_filter == 1 and mip_filter == 2:
            color0 = self._sample_linear(mip_levels[level0], tu, tv)
            color1 = self._sample_linear(mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 0 and mip_filter == 0:
            color0 = self._sample_nearest(mip_levels[level0], tu, tv)
            color1 = self._sample_nearest(mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 0 and mip_filter == 1:
            color0 = self._sample_nearest(mip_levels[level0], tu, tv)
            color1 = self._sample_nearest(mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 0 and mip_filter == 2:
            color0 = self._sample_linear(mip_levels[level0], tu, tv)
            color1 = self._sample_linear(mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 1 and mip_filter == 0:
            color0 = self._sample_linear(mip_levels[level0], tu, tv)
            color1 = self._sample_linear(mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 1 and mip_filter == 1:
            color0 = self._sample_linear(mip_levels[level0], tu, tv)
            color1 = self._sample_linear(mip_levels[level1], tu, tv)
        elif min_filter == 1 and mag_filter == 1 and mip_filter == 2:
            color0 = self._sample_linear(mip_levels[level0], tu, tv)
            color1 = self._sample_linear(mip_levels[level1], tu, tv)
        else:
            color0 = self._sample_linear(mip_levels[level0], tu, tv)
            color1 = self._sample_linear(mip_levels[level1], tu, tv)

        result = [
            color0[0] * (1 - s) + color1[0] * s,
            color0[1] * (1 - s) + color1[1] * s,
            color0[2] * (1 - s) + color1[2] * s,
            color0[3] * (1 - s) + color1[3] * s
        ]

        return [max(0.0, min(1.0, c)) for c in result]