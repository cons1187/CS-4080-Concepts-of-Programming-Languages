//example 1
//start clock
var a = 1;
var b = 2;
var c = 3;
// declare many more variables
//look them all up
print a;
print b;
print c;
//print time elapsed

//example 2
//start clock
class Point {
  init(x, y) {
    this.x = x;
    this.y = y;
  }
}
var p = Point(1, 2);
var i = 0;
while (i < 100000) {
  p.x = p.x + p.y;
  p.y = p.x - p.y;
  i = i + 1;
}
//print time elapsed

//example 3
//start clock
var a = "this is a test string";
var b = "this is a test string";
var i = 0;
while (i < 100000) {
  if (a == b) { i = i + 1; }
}
//print time elapsed