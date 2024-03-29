(module
	(func $fib (export "fib") (param i32) (result i32)
		(i32.gt_u (local.get 0) (i32.const 1))
		if (result i32)
			(i32.add
      			(call $fib (i32.sub (local.get 0) (i32.const 1)))
				(call $fib (i32.sub (local.get 0) (i32.const 2))))
		else
			local.get 0
	end)
)