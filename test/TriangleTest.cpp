#include "Triangle.h"
#include <math.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

/////////////////// FAIL TESTS ///////////////////////////////////////

// Test 1: Perimeter: Fails because getPerimeter() uses side2 twice
TEST(TriangleTests, TestPerimeterFail) {
    shapes::Triangle triangle(3, 2, 3);
    EXPECT_EQ(8, triangle.getPerimeter());

}

// Test 2: Area: Fails because uses getPerimeter
TEST(TriangleTests, TestAreaFail) {
    // Heronian Triangle Fail
    int side1 = 25;
    int side2 = 17;
    int side3 = 12;
    int area = 90;
    
    shapes::Triangle triangle(side1, side2, side3);
    EXPECT_EQ(area, (int)triangle.getArea());
}

// Test 3: Kind: Fails because checks for Isosceles before Equilateral
TEST(TriangleTests, TestKindFail) {
    shapes::Triangle triangle(5, 5, 5);
    EXPECT_EQ(shapes::Triangle::Kind::EQUILATERAL, triangle.getKind());
}

/////////////////// PASS TESTS ///////////////////////////////////////

// Test 1: Perimeter: Passes because getPerimeter() uses side2 twice
//                  and side2 = side3
TEST(TriangleTests, TestPerimeterPass) {
    shapes::Triangle triangle(5, 5, 5);
    EXPECT_EQ(15, triangle.getPerimeter());
}

// Test 2: Area: Passes because uses getPerimeter(), and equilateral
//              triangles are not caught by getPerimeter if side2 == side3
TEST(TriangleTests, TestAreaPass) {
    // Heronian Triangle Pass
    int side1 = 80;
    int side2 = 41;
    int side3 = 41;
    int area = 360;
    
    shapes::Triangle triangle(side1, side2, side3);
    EXPECT_EQ(area, (int)triangle.getArea());
}

// Test 3: Kind: Passes because checks for Isosceles before Equilateral,
//              and side1 = side2
TEST(TriangleTests, TestKindPass) {
    shapes::Triangle triangle(5, 5, 3);
    EXPECT_EQ(shapes::Triangle::Kind::ISOSCELES, triangle.getKind());
}