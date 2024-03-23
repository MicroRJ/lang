init = {1,2,3,4,5,6,7,8,9}
pf(init)

simp = fun() ? {
	local list = {}
	for i in 0..4 ? {
		local x = i
		local y = i*2
		local z = i*4

		list[i] = fun() ? {
			v = {}
			v.x = x
			v.y = y
			v.z = z
			leave v
		}
	}
	for i in 0..4 ? {
		pf(list[i]())
	}
}

simp()

for x in 0..4 ? {
	if x == 0 ? {
		pf(x, " if 0")
	} elif x == 1 ? {
		pf(x, " elif 1")
	} elif x == 2 ? {
		pf(x, " elif 2")
	} else {
		pf(x, " else")
	}
}

if 1 && 2 || 0 ? {
	pf("welcome")
} else {
	pf("not welcome");
}
test = fun(x,y) ? {
	if x != y ? {
		pf("FAIL: expected ", y, " instead got ", x)
	}
}
pf("begin")
test(0 && 0 && 0 && 0, 0)
test(0 || 0 || 0 || 0, 0)
test(0 && 0 || 0, 0)
test(0 && 0 || 1, 1)
test(0 && 1 || 2, 1)
test(0 && 0, 0)
test(0 && 1, 0)
test(1 && 1, 1)
test(0 || 0, 0)
test(0 || 1, 1)
test(1 || 1, 1)
test(0 && 0 || 0, 0)
_0 = fun() ? { pf("0") leave 0 }
_1 = fun() ? { pf("1") leave 1 }
_2 = fun() ? { pf("2") leave 2 }
_3 = fun() ? { pf("3") leave 3 }