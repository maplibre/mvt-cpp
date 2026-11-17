#include <mapbox/vector_tile.hpp>
#include <mapbox/vector_tile/version.hpp>
#include <protozero/pbf_writer.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include <catch.hpp>

static std::string open_tile(std::string const& path) {
    std::ifstream stream(path.c_str(),std::ios_base::in|std::ios_base::binary);
    if (!stream.is_open())
    {
        throw std::runtime_error("could not open: '" + path + "'");
    }
    std::string message(std::istreambuf_iterator<char>(stream.rdbuf()),(std::istreambuf_iterator<char>()));
    stream.close();
    return message;
}

#define ASSERT_KNOWN_FEATURE() \
    auto const layer_names = tile.layerNames(); \
    REQUIRE(layer_names.size() == 1); \
    REQUIRE(layer_names[0] == "layer_name"); \
    auto const layer = tile.getLayer("layer_name"); \
    REQUIRE(layer.featureCount() == 1); \
    REQUIRE(layer.getName() == "layer_name"); \
    auto const feature = mapbox::vector_tile::feature(layer.getFeature(0),layer); \
    auto const& feature_id = feature.getID(); \
    REQUIRE(feature_id.is<uint64_t>()); \
    REQUIRE(feature_id.get<uint64_t>() == 123ull); \
    auto props = feature.getProperties(); \
    auto itr = props.find("hello"); \
    REQUIRE(itr != props.end()); \
    auto const& val = itr->second; \
    REQUIRE(val.is<std::string>()); \
    REQUIRE(val.get<std::string>() == "world"); \
    auto opt_val = feature.getValue("hello"); \
    REQUIRE(opt_val.is<std::string>()); \
    REQUIRE(opt_val.get<std::string>() == "world"); \
    mapbox::vector_tile::points_arrays_type geom = feature.getGeometries<mapbox::vector_tile::points_arrays_type>(1.0);

std::string stringify_geom(mapbox::vector_tile::points_arrays_type const& geom) {
    std::stringstream s;
    for (auto const& point_array : geom) {
        s << "[";
        for (auto const& point : point_array) {
            s << point.x << ", " << point.y;
        }
        s << "]";
    }
    return s.str();
}

TEST_CASE( "Read Feature-single-point.mvt" ) {
    std::string buffer = open_tile("test/mvt-fixtures/fixtures/valid/Feature-single-point.mvt");
    mapbox::vector_tile::buffer tile(buffer);
    ASSERT_KNOWN_FEATURE()
    REQUIRE(feature.getType() == mapbox::vector_tile::GeomType::POINT); \
    REQUIRE(stringify_geom(geom) == "[25, 17]");
}

TEST_CASE( "Read Feature-single-multipoint.mvt" ) {
    std::string buffer = open_tile("test/mvt-fixtures/fixtures/valid/Feature-single-multipoint.mvt");
    mapbox::vector_tile::buffer tile(buffer);
    ASSERT_KNOWN_FEATURE()
    REQUIRE(feature.getType() == mapbox::vector_tile::GeomType::POINT); \
    REQUIRE(stringify_geom(geom) == "[5, 7][3, 2]");
}

TEST_CASE( "Read Feature-single-linestring.mvt" ) {
    std::string buffer = open_tile("test/mvt-fixtures/fixtures/valid/Feature-single-linestring.mvt");
    mapbox::vector_tile::buffer tile(buffer);
    ASSERT_KNOWN_FEATURE()
    REQUIRE(feature.getType() == mapbox::vector_tile::GeomType::LINESTRING); \
    REQUIRE(stringify_geom(geom) == "[2, 22, 1010, 10]");
}

TEST_CASE( "Read Feature-single-multilinestring.mvt" ) {
    std::string buffer = open_tile("test/mvt-fixtures/fixtures/valid/Feature-single-multilinestring.mvt");
    mapbox::vector_tile::buffer tile(buffer);
    ASSERT_KNOWN_FEATURE()
    REQUIRE(feature.getType() == mapbox::vector_tile::GeomType::LINESTRING); \
    REQUIRE(stringify_geom(geom) == "[2, 22, 1010, 10][1, 13, 5]");
}

TEST_CASE( "Read Feature-single-polygon.mvt" ) {
    std::string buffer = open_tile("test/mvt-fixtures/fixtures/valid/Feature-single-polygon.mvt");
    mapbox::vector_tile::buffer tile(buffer);
    ASSERT_KNOWN_FEATURE()
    REQUIRE(feature.getType() == mapbox::vector_tile::GeomType::POLYGON); \
    REQUIRE(stringify_geom(geom) == "[3, 68, 1220, 343, 6]");
}

/*TEST_CASE( "Read Feature-single-multipolygon.mvt" ) {
    std::string buffer = open_tile("test/mvt-fixtures/fixtures/valid/Feature-single-multipolygon.mvt");
    mapbox::vector_tile::buffer tile(buffer);
    ASSERT_KNOWN_FEATURE()
    REQUIRE(feature.getType() == mapbox::vector_tile::GeomType::POINT); \
    REQUIRE(stringify_geom(geom) == "25, 17");
}*/

TEST_CASE( "Prevent massive over allocation" ) {
    std::string buffer = open_tile("test/test046.mvt");
    mapbox::vector_tile::buffer tile(buffer);
    auto const layer_names = tile.layerNames();
    REQUIRE(layer_names.size() == 1);
    REQUIRE(layer_names[0] == "0000000000");
    auto const layer = tile.getLayer("0000000000");
    REQUIRE(layer.featureCount() == 1);
    REQUIRE(layer.getName() == "0000000000");
    auto const feature = mapbox::vector_tile::feature(layer.getFeature(0),layer);
    mapbox::vector_tile::points_arrays_type geom = feature.getGeometries<mapbox::vector_tile::points_arrays_type>(1.0);
    REQUIRE(geom.capacity() <= 655360);
}

TEST_CASE( "Prevent underflow in case of LineTo with 0 command count" ) {
    std::string buffer = open_tile("test/test2048.mvt");
    mapbox::vector_tile::buffer tile(buffer);
    auto const layer_names = tile.layerNames();
    REQUIRE(layer_names.size() == 1);
    REQUIRE(layer_names[0] == "roads");
    auto const layer = tile.getLayer("roads");
    REQUIRE(layer.featureCount() == 243);
    for (std::size_t i = 0; i < layer.featureCount(); ++i) {
        auto const feature = mapbox::vector_tile::feature(layer.getFeature(i), layer);
        REQUIRE(feature.getType() == mapbox::vector_tile::GeomType::LINESTRING);
        auto geom = feature.getGeometries<mapbox::vector_tile::points_arrays_type>(1.0);
        REQUIRE(!geom.empty());
    }
}

TEST_CASE( "Handle layer with duplicate keys" ) {
    std::string buffer = open_tile("test/test047.mvt");
    mapbox::vector_tile::buffer tile(buffer);
    auto const layer = tile.getLayer("hello");
    auto const feature = mapbox::vector_tile::feature(layer.getFeature(0),layer);
    auto opt_val = feature.getValue("type"); \
    REQUIRE(opt_val.is<std::string>()); \
    REQUIRE(opt_val.get<std::string>() == "lake"); \
}

TEST_CASE( "Clamp coordinates outside the range of coordinate_type" ) {
    // A linestring from (10, 10) to (40000, 10) to (-40000, -40000): valid per the spec,
    // which allows geometry beyond the tile extent, but outside the range of std::int16_t.
    std::vector<std::uint32_t> const geometry = {
        9, protozero::encode_zigzag32(10), protozero::encode_zigzag32(10),          // MoveTo(10, 10)
        18, protozero::encode_zigzag32(39990), protozero::encode_zigzag32(0),       // LineTo(40000, 10)
        protozero::encode_zigzag32(-80000), protozero::encode_zigzag32(-40010),     //   and (-40000, -40000)
    };

    std::string buffer;
    {
        protozero::pbf_writer tile_writer(buffer);
        protozero::pbf_writer layer_writer(tile_writer, 3);
        layer_writer.add_uint32(15, 2);
        layer_writer.add_string(1, "layer");
        {
            protozero::pbf_writer feature_writer(layer_writer, 2);
            feature_writer.add_enum(3, 2);
            feature_writer.add_packed_uint32(4, geometry.begin(), geometry.end());
        }
        layer_writer.add_uint32(5, 4096);
    }

    mapbox::vector_tile::buffer tile(buffer);
    auto const layer = tile.getLayer("layer");
    auto const feature = mapbox::vector_tile::feature(layer.getFeature(0), layer);

    auto const geom = feature.getGeometries<mapbox::vector_tile::points_arrays_type>(1.0);
    REQUIRE(geom.size() == 1);
    REQUIRE(geom[0].size() == 3);
    CHECK(geom[0][0] == mapbox::vector_tile::point_type(10, 10));
    CHECK(geom[0][1] == mapbox::vector_tile::point_type(32767, 10));
    CHECK(geom[0][2] == mapbox::vector_tile::point_type(-32768, -32768));

    // Scaled, as MapLibre Native does from an extent of 4096 to 8192.
    auto const scaled = feature.getGeometries<mapbox::vector_tile::points_arrays_type>(2.0);
    REQUIRE(scaled.size() == 1);
    REQUIRE(scaled[0].size() == 3);
    CHECK(scaled[0][0] == mapbox::vector_tile::point_type(20, 20));
    CHECK(scaled[0][1] == mapbox::vector_tile::point_type(32767, 20));
    CHECK(scaled[0][2] == mapbox::vector_tile::point_type(-32768, -32768));
}
