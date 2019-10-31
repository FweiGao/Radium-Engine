#include <Engine/Renderer/RenderObject/Primitives/DrawPrimitives.hpp>

#include <Core/Geometry/MeshPrimitives.hpp>
#include <Core/Utils/Color.hpp>

#include <Engine/Renderer/Mesh/Mesh.hpp>
#include <Engine/Renderer/RenderObject/RenderObject.hpp>
#include <Engine/Renderer/RenderTechnique/RenderTechnique.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderConfigFactory.hpp>

#include <Core/Containers/MakeShared.hpp>
#include <Engine/Renderer/Material/BlinnPhongMaterial.hpp>

#include <algorithm>
#include <numeric>

namespace Ra {

using namespace Core::Geometry;

namespace Engine {
namespace DrawPrimitives {
RenderObject* Primitive( Component* component, const MeshPtr& mesh ) {
    ShaderConfiguration config;
    if ( mesh->getRenderMode() == GL_LINES )
    { config = ShaderConfigurationFactory::getConfiguration( "Lines" ); }
    else
    { config = ShaderConfigurationFactory::getConfiguration( "Plain" ); }

    auto mat = Core::make_shared<BlinnPhongMaterial>( "Default material" );
    RenderTechnique rt;
    rt.setMaterial( mat );
    rt.setConfiguration( config );

    return RenderObject::createRenderObject(
        mesh->getName(), component, RenderObjectType::Debug, mesh, rt );
}

LineMeshPtr Point( const Core::Vector3& point, const Core::Utils::Color& color, Scalar scale ) {

    Ra::Core::Geometry::LineMesh geom;
    geom.setVertices( {( point + ( scale * Core::Vector3::UnitX() ) ),
                       ( point - ( scale * Core::Vector3::UnitX() ) ),

                       ( point + ( scale * Core::Vector3::UnitY() ) ),
                       ( point - ( scale * Core::Vector3::UnitY() ) ),

                       ( point + ( scale * Core::Vector3::UnitZ() ) ),
                       ( point - ( scale * Core::Vector3::UnitZ() ) )} );
    geom.m_indices = {{0, 1}, {2, 3}, {4, 5}};

    LineMeshPtr mesh( new LineMesh( "Point Primitive", std::move( geom ), Mesh::RM_LINES ) );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ),
                                       Core::Vector4Array{mesh->getNumVertices(), color} );

    return mesh;
}

LineMeshPtr
Line( const Core::Vector3& a, const Core::Vector3& b, const Core::Utils::Color& color ) {
    Ra::Core::Geometry::LineMesh geom;
    geom.setVertices( {a, b} );
    geom.m_indices = {{0, 1}};

    LineMeshPtr mesh( new LineMesh( "Line Primitive", std::move( geom ), Mesh::RM_LINES ) );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ),
                                       Core::Vector4Array{mesh->getNumVertices(), color} );

    return mesh;
}

LineMeshPtr
Vector( const Core::Vector3& start, const Core::Vector3& v, const Core::Utils::Color& color ) {
    Core::Vector3 end = start + v;
    Core::Vector3 a, b;
    Core::Math::getOrthogonalVectors( v.normalized(), a, b );
    a.normalize();
    Scalar l = v.norm();

    const Scalar arrowFract = 0.1f;

    Ra::Core::Geometry::LineMesh geom;
    geom.setVertices( {start,
                       end,
                       start + ( ( 1.f - arrowFract ) * v ) + ( ( arrowFract * l ) * a ),
                       start + ( ( 1.f - arrowFract ) * v ) - ( ( arrowFract * l ) * a )} );
    geom.m_indices = {{0, 1}, {1, 2}, {1, 3}};

    LineMeshPtr mesh( new LineMesh( "Vector Primitive", std::move( geom ), Mesh::RM_LINES ) );

    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ),
                                       Core::Vector4Array{mesh->getNumVertices(), color} );

    return mesh;
}

MeshPtr Ray( const Core::Ray& ray, const Core::Utils::Color& color, Scalar len ) {
    Core::Vector3 end = ray.pointAt( len );

    Core::Vector3Array vertices = {ray.origin(), end};
    std::vector<uint> indices   = {0, 1};

    Core::Vector4Array colors( vertices.size(), color );

    MeshPtr mesh( new Mesh( "Ray Primitive", Mesh::RM_LINES ) );
    mesh->loadGeometry( vertices, indices );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );

    return mesh;
}

MeshPtr Triangle( const Core::Vector3& a,
                  const Core::Vector3& b,
                  const Core::Vector3& c,
                  const Core::Utils::Color& color,
                  bool fill ) {
    Core::Vector3Array vertices = {a, b, c};
    std::vector<uint> indices;

    if ( fill ) { indices = {0, 1, 2}; }
    else
    { indices = {0, 1, 1, 2, 2, 0}; }

    Mesh::MeshRenderMode renderType = fill ? Mesh::RM_TRIANGLES : Mesh::RM_LINES;

    Core::Vector4Array colors( vertices.size(), color );

    MeshPtr mesh( new Mesh( "Triangle Primitive", renderType ) );
    mesh->loadGeometry( vertices, indices );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );

    return mesh;
}

MeshPtr QuadStrip( const Core::Vector3& a,
                   const Core::Vector3& x,
                   const Core::Vector3& y,
                   uint quads,
                   const Core::Utils::Color& color ) {
    Core::Vector3Array vertices( quads * 2 + 2 );
    std::vector<uint> indices( quads * 2 + 2 );

    Core::Vector3 B = a;
    vertices[0]     = B;
    vertices[1]     = B + x;
    indices[0]      = 0;
    indices[1]      = 1;
    for ( uint i = 0; i < quads; ++i )
    {
        B += y;
        vertices[2 * i + 2] = B;
        vertices[2 * i + 3] = B + x;
        indices[2 * i + 2]  = 2 * i + 2;
        indices[2 * i + 3]  = 2 * i + 3;
    }

    Core::Vector4Array colors( vertices.size(), color );

    MeshPtr mesh( new Mesh( "Quad Strip Primitive", Mesh::RM_TRIANGLE_STRIP ) );
    mesh->loadGeometry( vertices, indices );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );
    return mesh;
}

LineMeshPtr Circle( const Core::Vector3& center,
                    const Core::Vector3& normal,
                    Scalar radius,
                    uint segments,
                    const Core::Utils::Color& color ) {
    CORE_ASSERT( segments >= 2, "Cannot draw a circle with less than 3 points" );

    Ra::Core::Geometry::LineMesh geom;
    Core::Vector3Array vertices( segments + 1 );
    geom.m_indices.resize( segments );

    Core::Vector3 xPlane, yPlane;
    Core::Math::getOrthogonalVectors( normal, xPlane, yPlane );
    xPlane.normalize();
    yPlane.normalize();

    Scalar thetaInc( Core::Math::PiMul2 / Scalar( segments ) );
    Scalar theta( 0.0 );
    vertices[0] = center + radius * ( std::cos( theta ) * xPlane + std::sin( theta ) * yPlane );
    theta += thetaInc;

    for ( uint i = 1; i <= segments; ++i )
    {
        vertices[i] = center + radius * ( std::cos( theta ) * xPlane + std::sin( theta ) * yPlane );
        geom.m_indices[i - 1] = {i - 1, i};
        theta += thetaInc;
    }

    geom.setVertices( vertices );

    geom.addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ),
                    Core::Vector4Array{geom.vertices().size(), color} );

    LineMeshPtr mesh( new LineMesh( "Circle Primitive", Mesh::RM_LINES ) );
    mesh->loadGeometry( std::move( geom ) );
    return mesh;
}

LineMeshPtr CircleArc( const Core::Vector3& center,
                       const Core::Vector3& normal,
                       Scalar radius,
                       Scalar angle,
                       uint segments,
                       const Core::Utils::Color& color ) {

    Ra::Core::Geometry::LineMesh geom;
    Core::Vector3Array vertices( segments + 1 );
    geom.m_indices.resize( segments );

    Core::Vector3 xPlane, yPlane;
    Core::Math::getOrthogonalVectors( normal, xPlane, yPlane );
    xPlane.normalize();
    yPlane.normalize();

    Scalar thetaInc( 2 * angle / Scalar( segments ) );
    Scalar theta( 0.0 );
    vertices[0] = center + radius * ( std::cos( theta ) * xPlane + std::sin( theta ) * yPlane );
    theta += thetaInc;

    for ( uint i = 1; i <= segments; ++i )
    {
        vertices[i] = center + radius * ( std::cos( theta ) * xPlane + std::sin( theta ) * yPlane );
        geom.m_indices[i - 1] = {i - 1, i};

        theta += thetaInc;
    }

    geom.setVertices( vertices );
    geom.addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ),
                    Core::Vector4Array{geom.vertices().size(), color} );

    LineMeshPtr mesh( new LineMesh( "Arc Circle Primitive", Mesh::RM_LINES ) );
    mesh->loadGeometry( std::move( geom ) );

    return mesh;
}

MeshPtr Sphere( const Core::Vector3& center, Scalar radius, const Core::Utils::Color& color ) {
    TriangleMesh sphere = makeGeodesicSphere( radius, 2 );
    auto handle         = sphere.getAttribHandle<TriangleMesh::Point>( "in_position" );
    auto& vertices      = sphere.getAttrib<TriangleMesh::Point>( handle );
    auto& data          = vertices.getDataWithLock();

    std::for_each( data.begin(), data.end(), [center]( Core::Vector3& v ) { v += center; } );
    vertices.unlock();

    Core::Vector4Array colors( sphere.vertices().size(), color );

    MeshPtr mesh( new Mesh( "Sphere Primitive", Mesh::RM_LINES ) );
    mesh->loadGeometry( std::move( sphere ) );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );

    return mesh;
}

MeshPtr Capsule( const Core::Vector3& p1,
                 const Core::Vector3& p2,
                 Scalar radius,
                 const Core::Utils::Color& color ) {
    const Scalar l = ( p2 - p1 ).norm();

    TriangleMesh capsule = makeCapsule( l, radius );

    // Compute the transform so that
    // (0,0,-l/2) maps to p1 and (0,0,l/2) maps to p2

    const Core::Vector3 trans = 0.5f * ( p2 + p1 );
    Core::Quaternion rot =
        Core::Quaternion::FromTwoVectors( Core::Vector3{0, 0, l / 2}, 0.5f * ( p1 - p2 ) );

    Core::Transform t = Core::Transform::Identity();
    t.rotate( rot );
    t.pretranslate( trans );

    auto handle    = capsule.getAttribHandle<TriangleMesh::Point>( "in_position" );
    auto& vertices = capsule.getAttrib<TriangleMesh::Point>( handle );
    auto& data     = vertices.getDataWithLock();

    std::for_each( data.begin(), data.end(), [t]( Core::Vector3& v ) { v = t * v; } );

    vertices.unlock();

    Core::Vector4Array colors( capsule.vertices().size(), color );

    MeshPtr mesh( new Mesh( "Sphere Primitive", Mesh::RM_LINES ) );
    mesh->loadGeometry( std::move( capsule ) );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );

    return mesh;
}

MeshPtr Disk( const Core::Vector3& center,
              const Core::Vector3& normal,
              Scalar radius,
              uint segments,
              const Core::Utils::Color& color ) {
    CORE_ASSERT( segments > 2, "Cannot draw a circle with less than 3 points" );

    uint seg = segments + 1;
    Core::Vector3Array vertices( seg );
    std::vector<uint> indices( seg + 1 );

    Core::Vector3 xPlane, yPlane;
    Core::Math::getOrthogonalVectors( normal, xPlane, yPlane );
    xPlane.normalize();
    yPlane.normalize();

    Scalar thetaInc( Core::Math::PiMul2 / Scalar( segments ) );
    Scalar theta( 0.0 );

    vertices[0] = center;
    indices[0]  = 0;
    for ( uint i = 1; i < seg; ++i )
    {
        vertices[i] = center + radius * ( std::cos( theta ) * xPlane + std::sin( theta ) * yPlane );
        indices[i]  = i;

        theta += thetaInc;
    }
    indices[seg] = 1;

    Core::Vector4Array colors( vertices.size(), color );

    MeshPtr mesh( new Mesh( "Disk Primitive", Mesh::RM_TRIANGLE_FAN ) );
    mesh->loadGeometry( vertices, indices );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );

    return mesh;
}

MeshPtr Normal( const Core::Vector3& point,
                const Core::Vector3& normal,
                const Core::Utils::Color& color,
                Scalar scale ) {
    // Display an arrow (just like the Vector() function)
    // plus the normal plane.
    Core::Vector3 a, b;
    Core::Vector3 n = normal.normalized();
    Core::Math::getOrthogonalVectors( n, a, b );

    n                 = scale * n;
    Core::Vector3 end = point + n;
    a.normalize();
    b.normalize();

    const Scalar arrowFract = 0.1f;

    Core::Vector3Array vertices = {
        point,
        end,
        point + ( ( 1.f - arrowFract ) * n ) + ( ( arrowFract * scale ) * a ),
        point + ( ( 1.f - arrowFract ) * n ) - ( ( arrowFract * scale ) * a ),

        point + ( scale * a ),
        point + ( scale * b ),
        point - ( scale * a ),
        point - ( scale * b ),
    };

    Core::Vector4Array colors( vertices.size(), color );

    std::vector<uint> indices = {0, 1, 1, 2, 1, 3, 4, 5, 5, 6, 6, 7, 7, 4, 4, 6, 5, 7};

    MeshPtr mesh( new Mesh( "Normal Primitive", Mesh::RM_LINES ) );
    mesh->loadGeometry( vertices, indices );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );

    return mesh;
}

MeshPtr Frame( const Core::Transform& frameFromEntity, Scalar scale ) {
    // Frame is a bit different from the others
    // since there are 3 lines of different colors.
    Core::Vector3 pos = frameFromEntity.translation();
    Core::Vector3 x   = frameFromEntity.linear() * Core::Vector3::UnitX();
    Core::Vector3 y   = frameFromEntity.linear() * Core::Vector3::UnitY();
    Core::Vector3 z   = frameFromEntity.linear() * Core::Vector3::UnitZ();

    Core::Vector3Array vertices = {
        pos, pos + scale * x, pos, pos + scale * y, pos, pos + scale * z};

    std::vector<uint> indices = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5};

    Core::Vector4Array colors = {
        Core::Utils::Color::Red(),
        Core::Utils::Color::Red(),
        Core::Utils::Color::Green(),
        Core::Utils::Color::Green(),
        Core::Utils::Color::Blue(),
        Core::Utils::Color::Blue(),
    };

    MeshPtr mesh( new Mesh( "Frame Primitive", Mesh::RM_LINES_ADJACENCY ) );
    mesh->loadGeometry( vertices, indices );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );

    return mesh;
}

MeshPtr Grid( const Core::Vector3& center,
              const Core::Vector3& x,
              const Core::Vector3& y,
              const Core::Utils::Color& color,
              Scalar cellSize,
              uint res ) {

    CORE_ASSERT( res > 1, "Grid has to be at least a 2x2 grid." );
    Core::Vector3Array vertices;
    std::vector<uint> indices;

    Scalar halfWidth{( cellSize * res ) / 2.f};
    Core::Vector3 deltaPosX{cellSize * x};
    Core::Vector3 startPosX{center - halfWidth * x};
    Core::Vector3 endPosX{center + halfWidth * x};
    Core::Vector3 deltaPosY{cellSize * y};
    Core::Vector3 startPosY{center - halfWidth * y};
    Core::Vector3 endPosY{center + halfWidth * y};
    Core::Vector3 currentPosX{startPosX};
    for ( uint i = 0; i < res + 1; ++i )
    {
        vertices.push_back( startPosY + currentPosX );
        vertices.push_back( endPosY + currentPosX );
        indices.push_back( uint( vertices.size() ) - 2 );
        indices.push_back( uint( vertices.size() ) - 1 );
        currentPosX += deltaPosX;
    }

    Core::Vector3 currentPosY = startPosY;
    for ( uint i = 0; i < res + 1; ++i )
    {
        vertices.push_back( startPosX + currentPosY );
        vertices.push_back( endPosX + currentPosY );
        indices.push_back( uint( vertices.size() ) - 2 );
        indices.push_back( uint( vertices.size() ) - 1 );
        currentPosY += deltaPosY;
    }

    Core::Vector4Array colors( vertices.size(), color );

    MeshPtr mesh( new Mesh( "GridPrimitive", Mesh::RM_LINES ) );
    mesh->loadGeometry( vertices, indices );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );

    return mesh;
}

MeshPtr AABB( const Core::Aabb& aabb, const Core::Utils::Color& color ) {
    Core::Vector3Array vertices( 8 );

    for ( uint i = 0; i < 8; ++i )
    {
        vertices[i] = aabb.corner( static_cast<Core::Aabb::CornerType>( i ) );
    }

    std::vector<uint> indices = {
        0, 1, 1, 3, 3, 2, 2, 0, // Floor
        0, 4, 1, 5, 2, 6, 3, 7, // Links
        4, 5, 5, 7, 7, 6, 6, 4, // Ceil
    };

    Core::Vector4Array colors( vertices.size(), color );

    MeshPtr mesh( new Mesh( "AABB Primitive", Mesh::RM_LINES ) );
    mesh->loadGeometry( vertices, indices );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );

    return mesh;
}

MeshPtr OBB( const Obb& obb, const Core::Utils::Color& color ) {
    Core::Vector3Array vertices( 8 );

    for ( uint i = 0; i < 8; ++i )
    {
        vertices[i] = obb.worldCorner( i );
    }

    std::vector<uint> indices = {
        0, 1, 1, 3, 3, 2, 2, 0, // Floor
        4, 5, 5, 7, 7, 6, 6, 4, // Ceil
        0, 4, 1, 5, 2, 6, 3, 7, // Links
    };

    Core::Vector4Array colors( vertices.size(), color );

    MeshPtr mesh( new Mesh( "OBB Primitive", Mesh::RM_LINES ) );
    mesh->loadGeometry( vertices, indices );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );

    return mesh;
}

MeshPtr Spline( const Core::Geometry::Spline<3, 3>& spline,
                uint pointCount,
                const Core::Utils::Color& color,
                Scalar /*scale*/ ) {
    Core::Vector3Array vertices;
    vertices.reserve( pointCount );

    std::vector<uint> indices;
    indices.reserve( pointCount * 2 - 2 );

    Scalar dt = Scalar( 1 ) / Scalar( pointCount - 1 );
    for ( uint i = 0; i < pointCount; ++i )
    {
        Scalar t = dt * i;
        vertices.push_back( spline.f( t ) );
    }

    for ( uint i = 0; i < pointCount - 1; ++i )
    {
        indices.push_back( i );
        indices.push_back( i + 1 );
    }

    Core::Vector4Array colors( vertices.size(), color );

    MeshPtr mesh( new Mesh( "Spline Primitive", Mesh::RM_LINES ) );
    mesh->loadGeometry( vertices, indices );
    mesh->getCoreGeometry().addAttrib( Mesh::getAttribName( Mesh::VERTEX_COLOR ), colors );

    return mesh;
}
} // namespace DrawPrimitives
} // namespace Engine
} // namespace Ra
