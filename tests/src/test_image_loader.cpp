#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <iostream>
#include "image_loader.hpp"


namespace {
	void load_non_existent() {
		using namespace StackExposures;

		ImageLoader loader;
	    auto unused = loader.load_image("/no/such/image.jpg");
	}
}

TEST_CASE("Load non-raw image") {
	REQUIRE_THROWS_AS(load_non_existent(), std::runtime_error);
}

TEST_CASE("Low-level load non-existent image") {
	auto processor = std::make_shared<LibRaw>();
	auto status = processor->open_file("/no/such/image/jpg");
	CHECK(status < 0);
	CHECK(std::string(processor->strerror(status)) == std::string("Input/output error"));
}