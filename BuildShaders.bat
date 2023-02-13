for /r res/shader %%f in (*.hlsl) do (
    find /c "vs_main" %%f > nul && call %cd%\build\dxc\bin\x64\dxc.exe -T vs_6_7 -E vs_main -Fo %%~dpnf.vs.bin %%f
    find /c "ps_main" %%f > nul && call %cd%\build\dxc\bin\x64\dxc.exe -T ps_6_7 -E ps_main -Fo %%~dpnf.ps.bin %%f
    find /c "gs_main" %%f > nul && call %cd%\build\dxc\bin\x64\dxc.exe -T gs_6_7 -E gs_main -Fo %%~dpnf.gs.bin %%f
    find /c "ds_main" %%f > nul && call %cd%\build\dxc\bin\x64\dxc.exe -T ds_6_7 -E ds_main -Fo %%~dpnf.ds.bin %%f
    find /c "hs_main" %%f > nul && call %cd%\build\dxc\bin\x64\dxc.exe -T hs_6_7 -E hs_main -Fo %%~dpnf.hs.bin %%f
    find /c "cs_main" %%f > nul && call %cd%\build\dxc\bin\x64\dxc.exe -T cs_6_7 -E cs_main -Fo %%~dpnf.cs.bin %%f
    find /c "as_main" %%f > nul && call %cd%\build\dxc\bin\x64\dxc.exe -T as_6_7 -E as_main -Fo %%~dpnf.as.bin %%f
    find /c "ms_main" %%f > nul && call %cd%\build\dxc\bin\x64\dxc.exe -T ms_6_7 -E ms_main -Fo %%~dpnf.ms.bin %%f
)
