
# Load all English key-value pairs in order
$baseFile = "d:\Gw2 Projekte\FarmingTracker\src\localization_en.cpp"
$englishTranslations = @()
Get-Content $baseFile | ForEach-Object {
    if ($_ -match '^\s*\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}') {
        $englishTranslations += [PSCustomObject]@{
            Key = $matches[1]
            Value = $matches[2]
            Line = $_
        }
    }
}

Write-Host "Loaded $($englishTranslations.Count) English translation pairs" -ForegroundColor Green

# Process all other localization files
$otherFiles = Get-ChildItem "d:\Gw2 Projekte\FarmingTracker\src" -Filter "localization_*.cpp" | Where-Object { $_.Name -ne "localization_en.cpp" }

foreach ($file in $otherFiles) {
    Write-Host "`nProcessing $($file.Name)..." -ForegroundColor Cyan

    # Load existing translations from this file
    $existing = @{}
    Get-Content $file.FullName | ForEach-Object {
        if ($_ -match '^\s*\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}') {
            $existing[$matches[1]] = $matches[2]
        }
    }
    Write-Host "  Found $($existing.Count) existing translations"

    # Now we need to reconstruct the file!
    # We'll read line by line, and when we hit the translations array, we'll replace it completely
    $newContent = @()
    $inTranslations = $false
    $translationsStarted = $false

    Get-Content $file.FullName | ForEach-Object {
        $line = $_

        # Check if we're entering the translations function
        if (-not $translationsStarted -and $line -match '^\s*static\s+const\s+std::unordered_map<std::string,\s+const\s+char\*\s+Get.*Translations\(\)') {
            $newContent += $line
            $inTranslations = $true
            $translationsStarted = $true
        } elseif ($inTranslations -and $line -match '^\s*\{\s*"[^"]+"\s*,\s*"[^"]+"\s*\}') {
            # Skip existing translation lines, we'll rebuild them all
        } elseif ($inTranslations -and $line -match '^\s*\}\s*;') {
            # We've reached the end of translations, add all our new translations first!
            Write-Host "  Adding all translations..."
            foreach ($pair in $englishTranslations) {
                # Use existing value if available, else English
                $val = if ($existing.ContainsKey($pair.Key)) { $existing[$pair.Key] } else { $pair.Value }
                $newContent += "            {`"$($pair.Key)`", `"$val`"},"
            }
            $newContent += $line
            $inTranslations = $false
        } else {
            # Keep all other lines!
            $newContent += $line
        }
    }

    # Write the new content back to the file!
    $newContent | Set-Content -Path $file.FullName -Encoding UTF8 -NoNewline

    Write-Host "  Successfully updated $($file.Name)!" -ForegroundColor Green
}

Write-Host "`n🎉 All localization files are now complete and up to date!" -ForegroundColor Green
