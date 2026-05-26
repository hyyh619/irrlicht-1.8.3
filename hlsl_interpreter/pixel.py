from dataclasses import dataclass
from typing import Dict, Any, Optional, List


@dataclass
class Pixel:
    """
    Pixel object - represents a rasterized pixel with interpolated attributes
    """
    x: int                           # Screen x coordinate
    y: int                           # Screen y coordinate
    depth: float                     # Depth value (for z-test)
    color: Optional[List[float]]     # Interpolated color (RGBA)
    texcoord: Optional[List[float]]  # Interpolated texture coordinate
    texcoord2: Optional[List[float]] # Interpolated second texture coordinate
    normal: Optional[List[float]]    # Interpolated normal vector
    worldPos: Optional[List[float]]  # Interpolated world position
    attributes: Dict[str, Any]       # Additional interpolated attributes
    primitive_id: int                # ID of the primitive this pixel belongs to
    sample_index: int = 0            # Sample index for MSAA
    ps_output_color: Optional[List[float]] = None  # Output color from Pixel Shader

    def __post_init__(self):
        if self.attributes is None:
            self.attributes = {}

    def get_attribute(self, name: str) -> Any:
        """Get interpolated attribute by name"""
        return self.attributes.get(name)

    def set_attribute(self, name: str, value: Any):
        """Set interpolated attribute"""
        self.attributes[name] = value

    def to_dict(self) -> Dict[str, Any]:
        """Convert pixel to dictionary"""
        result = {
            'x': self.x,
            'y': self.y,
            'depth': self.depth,
            'primitive_id': self.primitive_id,
            'sample_index': self.sample_index
        }
        if self.color is not None:
            result['color'] = self.color
        if self.texcoord is not None:
            result['texcoord'] = self.texcoord
        if self.texcoord2 is not None:
            result['texcoord2'] = self.texcoord2
        if self.normal is not None:
            result['normal'] = self.normal
        if self.position is not None:
            result['position'] = self.position
        result['attributes'] = self.attributes
        return result

    @staticmethod
    def from_dict(data: Dict[str, Any]) -> 'Pixel':
        """Create Pixel from dictionary"""
        return Pixel(
            x=data.get('x', 0),
            y=data.get('y', 0),
            depth=data.get('depth', 0.0),
            color=data.get('color'),
            texcoord=data.get('texcoord'),
            texcoord2=data.get('texcoord2'),
            normal=data.get('normal'),
            position=data.get('position'),
            attributes=data.get('attributes', {}),
            primitive_id=data.get('primitive_id', 0),
            sample_index=data.get('sample_index', 0)
        )