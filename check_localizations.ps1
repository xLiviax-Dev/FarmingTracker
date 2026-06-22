
# Get base keys from localization_en.cpp
$baseFile = "d:\Gw2 Projekte\FarmingTracker\src\localization_en.cpp"
$baseKeys = @()
Get-Content $baseFile | ForEach-Object {
    if ($_ -match '^\s*\{\s*"([^"]+)"') {
        $baseKeys += $matches[1]
    }
}
$baseKeys = $baseKeys | Sort-Object -Unique

Write-Host "=== Found $($baseKeys.Count) keys in English ===" -ForegroundColor Green

# Get all other localization files
$otherFiles = Get-ChildItem "d:\Gw2 Projekte\FarmingTracker\src" -Filter "localization_*.cpp" | Where-Object { $_.Name -ne "localization_en.cpp" }

foreach ($file in $otherFiles) {
    Write-Host "`nChecking $($file.Name)..." -ForegroundColor Cyan

    $fileKeys = @()
    Get-Content $file.FullName | ForEach-Object {
        if ($_ -match '^\s*\{\s*"([^"]+)"') {
            $fileKeys += $matches[1]
        }
    }
    $fileKeys = $fileKeys | Sort-Object -Unique

    # Find missing keys
    $missing = @()
    if ($baseKeys) {
        $missing = $baseKeys | Where-Object { $_ -notin $fileKeys }
    }
    if ($missing) {
        Write-Host "❌ Missing keys: $($missing -join ', ')" -ForegroundColor Red
    } else {
        Write-Host "✅ All base keys present" -ForegroundColor Green
    }

    # Find extra keys
    $extra = @()
    if ($baseKeys) {
        $extra = $fileKeys | Where-Object { $_ -notin $baseKeys }
    }
    if ($extra) {
        Write-Host "⚠️ Extra keys: $($extra -join ', ')" -ForegroundColor Yellow
    } else {
        Write-Host "✅ No extra keys" -ForegroundColor Green
    }

    # Check count
    Write-Host "  Keys: $($fileKeys.Count) / $($baseKeys.Count)"
}
