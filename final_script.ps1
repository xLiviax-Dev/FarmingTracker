
# Load all English key-value pairs in order
$baseFile = "d:\Gw2 Projekte\FarmingTracker\src\localization_en.cpp"
$englishPairs = @()
Get-Content $baseFile | ForEach-Object {
    if ($_ -match '^\s*\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}') {
        $englishPairs += [PSCustomObject]@{
            Key = $matches[1]
            Value = $matches[2]
        }
    }
}
Write-Host "Loaded $($englishPairs.Count) English pairs" -ForegroundColor Green

# List of files to update (excluding en and de)
$filesToUpdate = @(
    "localization_cs.cpp",
    "localization_da.cpp",
    "localization_el.cpp",
    "localization_fi.cpp",
    "localization_hu.cpp",
    "localization_it.cpp",
    "localization_nl.cpp",
    "localization_no.cpp",
    "localization_pl.cpp",
    "localization_pt.cpp",
    "localization_ro.cpp",
    "localization_ru.cpp",
    "localization_sv.cpp",
    "localization_zh.cpp"
)

foreach ($fileName in $filesToUpdate) {
    $filePath = Join-Path "d:\Gw2 Projekte\FarmingTracker\src" $fileName
    Write-Host "`nProcessing $fileName" -ForegroundColor Cyan

    # Load existing translations
    $existing = @{}
    Get-Content $filePath | ForEach-Object {
        if ($_ -match '^\s*\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}') {
            $existing[$matches[1]] = $matches[2]
        }
    }
    Write-Host "Found $($existing.PSBase.Count) existing keys"

    # Now rebuild the file
    $newContent = @()
    $inTranslations = $false

    Get-Content $filePath | ForEach-Object {
        $line = $_

        if (-not $inTranslations -and $line -match '^\s*static\s+const\s+std::unordered_map<std::string,\s+const\s+char\*\s+Get.*Translations\(\)') {
            $newContent += $line
            $inTranslations = $true
        }
        elseif ($inTranslations -and $line -match '^\s*\{\s*"[^"]+"\s*,\s*"[^"]+"\s*\}') {
            # Skip, we'll add all in order
        }
        elseif ($inTranslations -and $line -match '^\s*\}\s*;') {
            # Add all English pairs in order, using existing translation if available
            foreach ($pair in $englishPairs) {
                $val = if ($existing.ContainsKey($pair.Key)) { $existing[$pair.Key] } else { $pair.Value }
                $newContent += "            {`"$($pair.Key)`", `"$val`"},"
            }
            $newContent += $line
            $inTranslations = $false
        }
        else {
            $newContent += $line
        }
    }

    # Write back to file, preserving line endings
    $newContent | Set-Content -Path $filePath -Encoding UTF8 -NoNewline

    Write-Host "Updated $fileName" -ForegroundColor Green
}

Write-Host "`nAll files updated!" -ForegroundColor Green
