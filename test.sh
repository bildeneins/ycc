#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./ycc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 51 "51;"
assert 21 "5+20-4;"
assert 13 "1 + 3 - 4 + 13;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 5 "-10+15;"
assert 100 "-10*-10;"
assert 1 "1<2;"
assert 0 "1==0;"
assert 1 "1!=2;"
assert 0 "0 > 0;"
assert 1 "1 + 1 <= 2;"
assert 1 "1 + 1 >= 2;"
assert 1 "a = 1;"
assert 4 "a = 1; b = 2; c = a * 2 + b; c;"
assert 6 "ya = 2; so = 3; ya * so;"
assert 2 "return 2; 1;"

echo OK