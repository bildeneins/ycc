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

assert 0 0
assert 51 51
assert 21 "5+20-4"
assert 13 "1 + 3 - 4 + 13"
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 4 '(3+5)/2'

echo OK

