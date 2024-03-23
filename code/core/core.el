/*
**
** Copyright(c) 2023 - Dayan Rodriguez - Dyr
**
** -+- elang -+-
**
*/

// TODO: Many more intrinsics
// TODO: function overloading
// TODO: Extensions (inheritance)
// TODO: Builtin Vectors and Matrices and Quaternions
// TODO: Replace 'operator' with 'binary' and 'unary'
// TODO: Think about constructors e.i `float4 :: struct (x := 0.,y := 0.,z := 0.,w := 0.) {}`

true  ::= 1
false ::= 0
PI ::= 3.1415
TAU ::= 6.2831853071

__err :: addon "lib_c" () -> int
__log :: addon "lib_c" (f: str) -> int
__fmt :: addon "lib_c" (f: str, ..) -> str

iton :: (i: num) -> num {
	ret i
}

ntoi :: (i: int) -> int {
	ret i
}

xorshift32 :: (q: int) -> int {
	q = q ^ q << 13
	q = q ^ q >> 17
	q = q ^ q << 5
	ret q
}

// mix :: intrinsic (x: float, y: float, t: float) -> float

// TODO: instrinsic
sat :: (val: num, low: num, high: num) -> float {
	ret max(low,min(val,high))
}

// TODO: instrinsic
mix :: (x: float, y: float, t: float) -> float {
	ret x + ((y - x) * t)
}

// TODO: DEPRECATED
wrap :: (val: int, min_: int, max_: int) -> int {
	ret min_ + iabs(val % (max_ - min_ + 1))
}

seed := 1509341093
rnd :: (min_: int, max_: int) -> int {
	seed = xorshift32(seed)
	ret wrap(seed,min_,max_)
}

rnd_n :: () -> float {
	ret iton(rnd(0,10000000)) / 10000000.
}

float4 :: struct {
	x := 0.
	y := 0.
	z := 0.
	w := 1.
}

vec2 :: struct {
	x := 0
	y := 0
}

int2 :: struct {
	x := 0
	y := 0
}

// TODO: switch to using local storage instead!
int2_add :: (a: int2 *, b: int2 *) -> int2 * {
	ret new int2 { a.x + b.x, a.y + b.y }
}

float2 :: struct {
	x := 0.
	y := 0.
}

angle_to_float2 :: (v :float) -> float2 {
	ret local float2 { cos(v), sin(v) }
}

float2_to_angle :: (v: float2) -> float {
	ret atan2(v.y,v.x)
}

float2_abs :: (v: float2) -> float2 {
	x ::= abs(v.x)
	y ::= abs(v.y)
	ret local float2 { x,y }
}

float2_max :: (v: float2, m: float) -> float2 {
	x ::= max(v.x,m)
	y ::= max(v.y,m)
	ret local float2 { x,y }
}

float2_min :: (v: float2, m: float) -> float2 {
	x ::= min(v.x,m)
	y ::= min(v.y,m)
	ret local float2 { x,y }
}

float2_sub :: operator - (i: float2, k: float2) -> float2 {
	ret local float2 { i.x - k.x, i.y - k.y }
}

float2_add :: operator + (i: float2, k: float2) -> float2 {
	ret local float2 { i.x + k.x, i.y + k.y }
}

float2_mul :: operator * (i: float2, k: float2) -> float2 {
	ret local float2 { i.x * k.x, i.y * k.y }
}

float2_scale :: operator * (i: float2, k: float) -> float2 {
	ret local float2 { i.x * k, i.y * k }
}

dot :: (a: float2, b: float2) -> float {
	ret (a.x * b.x) + (a.y * b.y)
}

float2_length :: (v: float2) -> float {
	ret sqrt(dot(v,v))
}

float2_rotate :: (v: float2, angle: float) -> float2 {
	rotated_x ::= v.x * cos(angle) - v.y * sin(angle)
	rotated_y ::= v.x * sin(angle) + v.y * cos(angle)
	ret local float2{rotated_x,rotated_y}
}

float2_random_normal :: () -> float2 {
	angle ::= mix(-PI,+PI,rnd_n())
	x ::= cos(angle)
	y ::= sin(angle)
	ret local float2{x,y}
}

float2_lerp :: (v: float2, k: float2, t: float) -> float2 {
	ret local float2 { mix(v.x,k.x,t), mix(v.y,k.y,t) }
}

rnd_dir :: (base := local float2{0,1}, range_min := -PI, range_max := +PI) -> float2 {
	angle ::= atan2(base.y,base.x) + mix(range_min,range_max,rnd_n())
	x ::= cos(angle)
	y ::= sin(angle)
	ret local float2{x,y}
}

cross :: (v: float2) -> float2 {
	x ::= + v.y
	y ::= - v.x
	ret local float2{x,y}
}

float2_clamp_to_length :: (v: float2, m: float) -> float2 {
	l ::= float2_length(v)
	if m < l  ? {
		s ::= m / l
		ret local float2 { v.x * s, v.y * s }
	}
	ret local float2 { v.x, v.y }
}

normalize :: (v: float2) -> float2 {
	l ::= float2_length(v)
	ret local float2 { v.x / l, v.y / l }
}

reflect :: (n: float2, i: float2) -> float2 {
	d ::= 2. * dot(n,i)
	ret local float2 {
		i.x - (d * n.x),
		i.y - (d * n.y),
	}
}

negate :: (i: float2) -> float2 {
	ret local float2 { - i.x, - i.y }
}

add :: (a: vec2 *, b: vec2 *) -> vec2 * {
	ret local vec2 { a.x + b.x, a.y + b.y }
}

boxsdf :: (d: float2, b: float2, r := 0.) -> float {
	q ::= float2_sub(float2_abs(d),float2_sub(b,local float2{r,r}))
	ret float2_length(float2_max(q,.0)) + min(max(q.x,q.y),.0) - r
}

// boxsdf :: (q: float2) -> float {
// 	ret float2_length(float2_max(q,0.)) + min(max(q.x,q.y),0.0)
// }

	test_return_large_item :: () -> float2 {
		ret local float2{33,33}
	}
	test_spill_forward :: () -> float {
		for i in 0..100 ? {
			local float2{}
			local float2{}
			local float2{}
			local float2{}
		}
		ret 0
	}
	test_spill_backwards :: () -> float {
		for i in 0..1000 ? {
			thing ::= test_return_large_item()
			if thing.x != 33. ? {
				__err()
			}
			if thing.y != 33. ? {
				__err()
			}
		}
		ret 0
	}

	float2_neq :: (a: float2, b: float2) -> int {
		if a.x != b.x ? {
			ret 1
		}
		if a.y != b.y ? {
			ret 1
		}
		ret 0
	}

	testelang :: () -> int {
		__log("testing elang")

		a ::= local float2{1,1}
		b ::= local float2{1,1}

		test_result_0 ::= float2_add(a,b)
		test_result_1 ::= float2_add(a,b)

		inc ::= local float2{0,0}
		for i in 0..256 ? {
			last ::= float2_add(inc,local float2{44,44})
			inc = float2_add(inc,last)
		}

		for i in 0..256 ? {
			if float2_neq(local float2{2,2},float2_add(a,a)) ? {
				__err()
			}
			if float2_neq(local float2{0,0},float2_sub(a,a)) ? {
				__err()
			}
			if float2_neq(a,float2_mul(a,a)) ? {
				__err()
			}
		}

		for i in 0..100 ? {
			j ::= i
			k ::= i
			thestring ::= __fmt("something %",i)
		}

		test_spill_forward()
		test_spill_backwards()

		array := new int[1000]
		if lengthOf(array) != 1000 ? {
			__err()
		}
		for i in 0..1000 ? {
			array[i] = i
		}
		x := 0
		for x < 1000 ? {
			if array[x] != x ? {
				__err()
			}
			x = x + 1
		}
		for j in 0..lengthOf(array) ? {
			array[j] = 0
		}
		j := 0
		for j < 1000 ? {
			if array[j] != 0 ? {
				__err()
			}
			j = j + 1
		}
		if x != lengthOf(array) ? {
			__err()
		}
		ret 0
	}