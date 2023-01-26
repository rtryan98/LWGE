New-Item -ItemType Directory -Force -Path thirdparty/d3d12agilitysdk/agilitysdk
New-Item -ItemType Directory -Force -Path thirdparty/DirectXShaderCompiler/dxc
New-Item -ItemType Directory -Force -Path build/D3D12AgilitySDK
New-Item -ItemType Directory -Force -Path build/Debug/D3D12
New-Item -ItemType Directory -Force -Path build/RelWithDebInfo/D3D12
New-Item -ItemType Directory -Force -Path build/Release/D3D12

Invoke-WebRequest -Uri https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.608.2 -OutFile build/agility.zip
Expand-Archive -Path build/agility.zip -DestinationPath build/D3D12AgilitySDK -Force
Copy-Item -Path build/D3D12AgilitySDK/build/native/include/* -Destination thirdparty/d3d12agilitysdk/agilitysdk -Force
Copy-Item -Path build/D3D12AgilitySDK/build/native/bin/x64/* -Destination build/Debug/D3D12 -Force
Copy-Item -Path build/D3D12AgilitySDK/build/native/bin/x64/* -Destination build/RelWithDebInfo/D3D12 -Force
Copy-Item -Path build/D3D12AgilitySDK/build/native/bin/x64/* -Destination build/Release/D3D12 -Force
Remove-Item -Path build/agility.zip

Invoke-WebRequest -Uri https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2207/dxc_2022_07_18.zip -OutFile build/dxc_2022_07_18.zip
Expand-Archive -Path build/dxc_2022_07_18.zip -DestinationPath build/dxc -Force
Copy-Item -Path build/dxc/inc/* -Destination thirdparty/DirectXShaderCompiler/dxc -Force
Copy-Item -Path build/dxc/lib/x64/* -Destination thirdparty/DirectXShaderCompiler/dxc -Force
Copy-Item -Path build/dxc/bin/x64/* -Destination build/Debug -Force
Copy-Item -Path build/dxc/bin/x64/* -Destination build/RelWithDebInfo -Force
Copy-Item -Path build/dxc/bin/x64/* -Destination build/Release -Force
Remove-Item -Path build/dxc_2022_07_18.zip
