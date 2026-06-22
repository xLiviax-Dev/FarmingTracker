
# Load all English key-value pairs in order
$baseFile = "d:\Gw2 Projekte\FarmingTracker\src\localization_en.cpp"
$englishTranslations = @()
Get-Content $baseFile | ForEach-Object {
    if ($_ -match '^\s*\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}') {
        $englishTranslations += [PSCustomObject]@{
            Key = $matches[1]
            Value = $matches[2]
        }
    }
}

Write-Host "Loaded $($englishTranslations.Length) English translation pairs" -ForegroundColor Green

# Process all other localization files
$otherFiles = Get-ChildItem "d:\Gw2 Projekte\FarmingTracker\src" -Filter "localization_*.cpp" | Where-Object { $_.Name -ne "localization_en.cpp" }

foreach ($file in $otherFiles) {
    Write-Host "`nProcessing $($file.Name)..." -ForegroundColor Cyan

    # Load existing translations into an array!
    $existingKeyValues = @()
    Get-Content $file.FullName | ForEach-Object {
        if ($_ -match '^\s*\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}') {
            $existingKeyValues += [PSCustomObject]@{
                Key = $matches[1]
                Value = $matches[2]
            }
        }
    }
    Write-Host "  Found $($existingKeyValues.Length) existing translations"

    # Now create a lookup array with all keys
    $existingLookup = @{}
    foreach ($item in $existingKeyValues) {
        $existingLookup[$item.Key] = $item.Value
    }
    # To get the count safely, use $existingLookup.Keys.Count
    Write-Host "  Lookup has $($existingLookup.Keys.Count) unique keys"

    # Now reconstruct the file!
    $newContent = @()
    $inTranslations = $false

    Get-Content $file.FullName | ForEach-Object {
        $line = $_

        # Check if we're entering the translations function
        if (-not $inTranslations -and $line -match '^\s*static\s+const\s+std::unordered_map<std::string,\s+const\s+char\*\s+Get.*Translations\(\)') {
            $newContent += $line
            $inTranslations = $true
        } elseif ($inTranslations -and $line -match '^\s*\{\s*"[^"]+"\s*,\s*"[^"]+"\s*\}') {
            # Skip existing translation lines
        } elseif ($inTranslations -and $line -match '^\s*\}\s*;') {
            # Now add all translations!
            foreach ($pair in $englishTranslations) {
                if ($existingLookup.ContainsKey($pair.Key)) {
                    $val = $existingLookup[$pair.Key]
                } else {
                    $val = $pair.Value
                }
                $newContent += "            {`"$($pair.Key)`", `"$val`"},"
            }
            $newContent += $line
            $inTranslations = $false
        } else {
            $newContent += $line
        }
    }

    # Write it back
    $newContent | Set-Content -Path $file.FullName -Encoding UTF8 -NoNewline
    Write-Host "  Updated $($file.Name)!" -ForegroundColor Green
}
Write-Host "`n✅ All localization files are now complete!" -ForegroundColor Green
