
local j = fun () ? pf("you've called j")
j()

local a = fun() ? {
	local x = 0
	local b = fun() ? {
		local x = 0
	}
}

for x in 0..4 ? {
	  if x == 0 ? pf(x, ": if 0")
	elif x == 1 ? pf(x, ": elif 1")
	elif x == 2 ? pf(x, ": elif 2")
	else pf(x, ": else")
}