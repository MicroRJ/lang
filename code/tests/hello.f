// listdir takes a function used for filtering!
// C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build

sysfile = fopen("hashbench.txt","ab")
fpf(sysfile,"Hash table benchmarks:\n")

run = fun(x) ? {
	gcpause()
	pf("filtering files in: ", x)
	_time = clocktime()
	local list = listdir(x,fun(file,flags) ? {
		if 512 < flags.n ? {
			leave false
		}
		flags.n = flags.n + 1;
		// fpf(sysfile,"file: ",file.name,"\n")
		if file.name:match("*.*") ? {
			// if file.name:match("vcvars*.bat") ? {
			// pf("found: ", file.name)
			0
		}
		leave true
	})
	fpf(sysfile,"elapsed: ", timediffs(_time), "\n")
	fpf(sysfile,"items: ", list:length, ", hits: ", list:collisions, "\n")
	fpf(sysfile,"====================================\n")
	// local filesize = fsize(sysfile)
	// pf("wrote: ", filesize, " bytes")
}

// run("C:/Program Files/Microsoft Visual Studio")
// run("C:/Program Files")
run("C:")
