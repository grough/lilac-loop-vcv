#include "catch.hpp"
#include "buffer.hpp"

TEST_CASE("RBuf writes and reads and wraps around", "[]") {
  RBuf rb(4);
  for (size_t i = 0; i < 4; i++) {
    rb.write((i + 1) * 1000.f);
    REQUIRE(rb.read() == Approx((i + 1) * 1000.f));
    rb.next();
  }
  REQUIRE(rb.read() == Approx(1000.f));
  rb.next();
  REQUIRE(rb.read() == Approx(2000.f));
  rb.next();
  REQUIRE(rb.read() == Approx(3000.f));
  rb.next();
  REQUIRE(rb.read() == Approx(4000.f));
  rb.next();
  REQUIRE(rb.read() == Approx(1000.f));
}

TEST_CASE("RBuf can read forwards and backwards", "[]") {
  RBuf rb(16);
  for (size_t i = 0; i < 16; i++) {
    rb.write((i + 1) * 1000.f);
    REQUIRE(rb.read() == Approx((i + 1) * 1000.f));
    rb.next();
  }
  REQUIRE(rb.read() == Approx(1000.f));
}

TEST_CASE("LoopBuf (2) grows when recording and loops around when not recording", "[]") {
  LoopBuf lb;
  int size = 2;
  lb.open = true;
  lb.process(size, 10.f);
  REQUIRE(lb.samples.size() == 1);
  REQUIRE(lb.read() == 10.f);
  lb.process(size, 20.f);
  REQUIRE(lb.samples.size() == 2);
  REQUIRE(lb.read() == 20.f);
  lb.process(size, 30.f);
  REQUIRE(lb.samples.size() == 2);
  REQUIRE(lb.read() == 10.f);
  lb.process(size, 40.f);
  REQUIRE(lb.samples.size() == 2);
  REQUIRE(lb.read() == 20.f);
}