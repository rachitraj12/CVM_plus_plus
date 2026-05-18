# PowerShell test runner for CVM++
# Runs all .cvm files in the tests/ directory using build\cvm.exe

$exe = Join-Path -Path (Get-Location) -ChildPath "build\cvm.exe"
if (-not (Test-Path $exe)) {
    Write-Error "Executable not found: $exe. Build the project first (see README)."
    exit 1
}

$tests = Get-ChildItem -Path (Join-Path (Get-Location) 'tests') -Filter '*.cvm' | Sort-Object Name
foreach ($t in $tests) {
    Write-Host "=======================================";
    Write-Host "Running: $($t.Name)";
    Write-Host "=======================================";
    if ($t.Name -eq 'test_input.cvm') {
        # Create a temporary file with the sample input and redirect stdin from it using cmd
        $tmp = [System.IO.Path]::GetTempFileName()
        Set-Content -Path $tmp -Value "7"
        $exeEsc = '"' + $exe + '"'
        $testEsc = '"' + $t.FullName + '"'
        $tmpEsc = '"' + $tmp + '"'
        # Pipe the temp file contents into the executable to supply stdin
        Get-Content $tmp | & $exe $t.FullName
        Remove-Item $tmp -ErrorAction SilentlyContinue
    } else {
        & $exe $t.FullName
    }
    Write-Host "";
}

Write-Host "All tests executed.";
exit 0
