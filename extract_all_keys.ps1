
# Extract all key-value pairs from localization_en.cpp in order
$baseFile = "d:\Gw2 Projekte\FarmingTracker\src\localization_en.cpp"
$allPairs = @()

Get-Content $baseFile | ForEach-Object {
    if ($_ -match '^\s*\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}') {
        $key = $matches[1]
        $value = $matches[2]
        $allPairs += [PSCustomObject]@{
            Key = $key
            Value = $value
        }
    }
}

# Write to a CSV file for reference
$allPairs | Export-Csv -Path "d:\Gw2 Projekte\FarmingTracker\all_keys.csv" -NoTypeInformation -Encoding UTF8

Write-Host "Extracted $($allPairs.Count) keys from localization_en.csv!"
Write-Host "Saved to all_keys.csv"
