@IF "%INCLUDE%"=="" (
	@CALL vcvars64
)
@SETLOCAL
@REM /Qstrip_reflect /Qstrip_priv /Qstrip_debug
@SET mySources=..\..\lgi.hlsl
@REM @SET compilerOptions=/O3
@SET compilerOptions=/WX /Od
@SET myOptions=/nologo /Zi /Zss
@PUSHD "src\hlsl"
@REM // /Vn
@CALL fxc %myOptions% /T vs_5_0 /E MainVS %compilerOptions% /Fh lgi.vs.h  %mySources%
@CALL fxc %myOptions% /T ps_5_0 /E MainPS %compilerOptions% /Fh lgi.ps.h  %mySources%

@CALL fxc %myOptions% /T vs_5_0 /E MainVS_SDF       %compilerOptions% /Fh lgi_sdf.vs.h          %mySources%
@CALL fxc %myOptions% /T ps_5_0 /E MainPS_CircleSDF %compilerOptions% /Fh lgi_sdf_cir.ps.h  %mySources%
@CALL fxc %myOptions% /T ps_5_0 /E MainPS_BoxSDF    %compilerOptions% /Fh lgi_sdf_box.ps.h  %mySources%

@CALL fxc %myOptions% /T ps_5_0 /E MainPS_Text  %compilerOptions% /Fh lgi_txt.ps.h  %mySources%
@CALL fxc %myOptions% /T ps_5_0 /E MainPS_TextSDF  %compilerOptions% /Fh lgi_txt_sdf.ps.h  %mySources%
@POPD

