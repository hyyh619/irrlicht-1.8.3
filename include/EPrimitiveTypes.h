// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __E_PRIMITIVE_TYPES_H_INCLUDED__
#define __E_PRIMITIVE_TYPES_H_INCLUDED__

namespace irr
{
    namespace scene
    {
        //! Enumeration for all primitive types there are.
        enum E_PRIMITIVE_TYPE
        {
            //! All vertices are non-connected points.
            EPT_POINTS = 0,

            //! All vertices form a single connected line.
            EPT_LINE_STRIP,

            //! Just as LINE_STRIP, but the last and the first vertex is also connected.
            EPT_LINE_LOOP,

            //! Every two vertices are connected creating n/2 lines.
            EPT_LINES,

            //! After the first two vertices each vertex defines a new triangle.
            //! Always the two last and the new one form a new triangle.
            EPT_TRIANGLE_STRIP,

            //! After the first two vertices each vertex defines a new triangle.
            //! All around the common first vertex.
            EPT_TRIANGLE_FAN,

            //! Explicitly set all vertices for each triangle.
            EPT_TRIANGLES,

            //! After the first two vertices each further tw vetices create a quad with the preceding two.
            EPT_QUAD_STRIP,

            //! Every four vertices create a quad.
            EPT_QUADS,

            //! Just as LINE_LOOP, but filled.
            EPT_POLYGON,

            //! The single vertices are expanded to quad billboards on the GPU.
            EPT_POINT_SPRITES
        };

        static inline const c8* getPrimitiveTypeName(E_PRIMITIVE_TYPE pType)
        {
            switch (pType)
            {
                case EPT_POINTS: return "EPT_POINTS";
                case EPT_LINE_STRIP: return "EPT_LINE_STRIP";
                case EPT_LINE_LOOP: return "EPT_LINE_LOOP";
                case EPT_LINES: return "EPT_LINES";
                case EPT_TRIANGLE_STRIP: return "EPT_TRIANGLE_STRIP";
                case EPT_TRIANGLE_FAN: return "EPT_TRIANGLE_FAN";
                case EPT_TRIANGLES: return "EPT_TRIANGLES";
                case EPT_QUAD_STRIP: return "EPT_QUAD_STRIP";
                case EPT_QUADS: return "EPT_QUADS";
                case EPT_POLYGON: return "EPT_POLYGON";
                case EPT_POINT_SPRITES: return "EPT_POINT_SPRITES";
                default: return "?";
            }
        }
    } // end namespace scene
} // end namespace irr
#endif