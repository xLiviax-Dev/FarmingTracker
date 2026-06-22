
# Read all English key-value pairs in order
$baseFile = "d:\Gw2 Projekte\FarmingTracker\src\localization_en.cpp"
$englishPairs = @()
Get-Content $baseFile | ForEach-Object {
    if ($_ -match '^\s*\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}') {
        $key = $matches[1]
        $value = $matches[2]
        $englishPairs += [PSCustomObject]@{
            Key = $key
            Value = $value
            Line = $_
        }
    }
}
Write-Host "Loaded $($englishPairs.Count) English key-value pairs!" -ForegroundColor Green

# Get all other localization files
$otherFiles = Get-ChildItem "d:\Gw2 Projekte\FarmingTracker\src" -Filter "localization_*.cpp" | Where-Object { $_.Name -ne "localization_en.cpp" }
Write-Host "Found $($otherFiles.Count) other localization files to update" -ForegroundColor Cyan

foreach ($file in $otherFiles) {
    Write-Host "`nProcessing $($file.Name)..." -ForegroundColor Cyan
    
    # Read current file's content
    $content = Get-Content $file.FullName -Raw
    $currentKeys = @{}
    Get-Content $file.FullName | ForEach-Object {
        if ($_ -match '^\s*\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}') {
            $currentKeys[$matches[1]] = $matches[2]
        }
    }
    Write-Host "  Current keys: $($currentKeys.Count)"

    # Now, let's find the position where we need to insert missing keys
    # We'll just regenerate the file content, keeping existing pairs, inserting missing ones
    $newContent = @()
    $processed = @{}
    Get-Content $file.FullName | ForEach-Object {
        if ($_ -match '^\s*\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}') {
            $key = $matches[1]
            $processed[$key] = $true
            $newContent += $_
        } else {
            $newContent += $_
        }
    }
    # Now add the missing keys at the end of the translations array (before the closing })
    Write-Host "  Missing keys: $($englishPairs.Count - $processed.Count)"
    $missingKeys = $englishPairs | Where-Object { -not $processed.ContainsKey($_.Key) }
    # Find where to insert? Wait, let's find the line before the last }, so we insert before the end
    # But wait let's read the file line by line and add missing keys in order
    # Let's rebuild the file properly: we'll go through englishPairs in order
    $newFileContent = @()
    $inTranslations = $false
    $foundEnd = $false
    Get-Content $file.FullName | ForEach-Object {
        if (-not $inTranslations -and $_ -match '^\s*static\s+const\s+std::unordered_map<std::string,\s+const\s+char\*\s+Get.*Translations\(\)') {
            $inTranslations = $true
        }
        if (-not $foundEnd -and $_ -match '^\s*\}\s*;') {
            # We found the end of the translations array, first add the missing keys!
            foreach ($pair in $missingKeys) {
                $newFileContent += "            {`"$($pair.Key)`", `"$($pair.Value)`"},"
            }
            $foundEnd = $true
        }
        $newFileContent += $_
    }

    # Write the new content back to the file
    $newFileContent | Set-Content -Path $file.FullName -Encoding UTF8 -NoNewline
    Write-Host "  Updated $($file.Name)!" -ForegroundColor Green
}

Write-Host "`nAll localization files updated!" -ForegroundColor Green
