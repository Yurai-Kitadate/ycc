#!/bin/bash
compile() {
  cc -o ycc ycc.c
}
assert() {
  expected="$1"
  input="$2"
  ./ycc "$input" >tmp.s
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
compile
assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 4 '(3+5)/2'
assert 4 '-3*2 + 10'
assert 9 '(-3)*(-3)'
assert 0 '1 == 2'
assert 1 '1 == 1'
assert 3 '(1 == 1)*3 + (2 == 1)'
assert 4 '(1 <= 1)*3 + (2 > 1)'
echo OK
