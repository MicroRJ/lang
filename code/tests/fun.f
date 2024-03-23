
call = fun(x) ? { leave x() }

a = fun () ? {
	pf("a")
	b = fun () ? {
		pf("b")
		c = fun () ? {
			pf("c")
		}
		c()
	}
	b()
}
a()

xyz = fun(x,y,z) ? {
	pf("called xyz",x,y,z)
	call(fun () ? {
		pf("inner xyz",x,y,z)
	})
}

xyz("x","y","z")