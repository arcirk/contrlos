$is_exists = Test-Path -Path .\controls\cmake-build-debug\controls.dll
if($is_exists){
    Copy-Item -Path .\controls\cmake-build-debug\controls.dll -Destination .\TestControls\cmake-build-debug\controls.dll -Force
    Copy-Item -Path .\controls\cmake-build-debug\controls.dll -Destination .\profile_manager\cmake-build-debug\controls.dll -Force
    Copy-Item -Path .\controls\cmake-build-debug\controls.dll -Destination .\query_builder\cmake-build-debug\controls.dll -Force
}

$is_exists = Test-Path -Path .\tree_model\cmake-build-debug\tree_model.dll
if($is_exists){Copy-Item -Path .\tree_model\cmake-build-debug\tree_model.dll -Destination .\TestControls\cmake-build-debug\tree_model.dll -Force}
