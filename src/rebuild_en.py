import re

# Read German file to get the reference keys
with open('localization_de.cpp', 'r', encoding='utf-8') as f:
    de_content = f.read()

# Extract all key-value pairs from German file
de_entries = re.findall(r'{\"([^\"]+)\",\s*\"([^\"]+)\"}', de_content)
print(f'German entries: {len(de_entries)}')

# Create a dictionary from German entries
de_dict = dict(de_entries)

# Read old English file
with open('localization_en_old.cpp', 'r', encoding='utf-8') as f:
    en_old_content = f.read()

# Extract all key-value pairs from old English file
en_old_entries = re.findall(r'{\"([^\"]+)\",\s*\"([^\"]+)\"}', en_old_content)
print(f'Old English entries: {len(en_old_entries)}')

# Create a dictionary from old English entries (first occurrence wins)
en_old_dict = {}
for key, value in en_old_entries:
    if key not in en_old_dict:
        en_old_dict[key] = value

print(f'Unique Old English keys: {len(en_old_dict)}')

# Build new English content based on German keys
new_en_lines = []
new_en_lines.append('// ---------------------------------------------------------------------------')
new_en_lines.append('// localization_en.cpp – English translations for Farming Tracker')
new_en_lines.append('// ---------------------------------------------------------------------------')
new_en_lines.append('')
new_en_lines.append('#include "localization.h"')
new_en_lines.append('#include <unordered_map>')
new_en_lines.append('')
new_en_lines.append('namespace Localization')
new_en_lines.append('{')
new_en_lines.append('    const std::unordered_map<std::string, const char*> GetEnglishTranslations()')
new_en_lines.append('    {')
new_en_lines.append('        static const std::unordered_map<std::string, const char*> translations = {')

# Add entries in the same order as German file
for key, de_value in de_entries:
    if key in en_old_dict:
        en_value = en_old_dict[key]
    else:
        en_value = de_value  # Use German translation as fallback
        print(f'Missing English translation for key: {key}')
    new_en_lines.append(f'            {{\"{key}\", \"{en_value}\"}},')

new_en_lines.append('        };')
new_en_lines.append('        return translations;')
new_en_lines.append('    }')
new_en_lines.append('}')

# Write new English file
with open('localization_en_new.cpp', 'w', encoding='utf-8') as f:
    f.write('\n'.join(new_en_lines))

print(f'New English file created with {len(de_entries)} entries')
