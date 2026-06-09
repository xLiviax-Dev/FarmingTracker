import re

# Read German file to get the reference keys
with open('localization_de.cpp', 'r', encoding='utf-8') as f:
    de_content = f.read()

# Extract all key-value pairs from German file
de_entries = re.findall(r'{\"([^\"]+)\",\s*\"([^\"]+)\"}', de_content)
print(f'German entries: {len(de_entries)}')

# Create a dictionary from German entries (preserving order)
de_dict = {}
for key, value in de_entries:
    de_dict[key] = value

# List of language files to process
languages = [
    ('localization_cs.cpp', 'Czech'),
    ('localization_da.cpp', 'Danish'),
    ('localization_el.cpp', 'Greek'),
    ('localization_es.cpp', 'Spanish'),
    ('localization_fi.cpp', 'Finnish'),
    ('localization_fr.cpp', 'French'),
    ('localization_hu.cpp', 'Hungarian'),
    ('localization_it.cpp', 'Italian'),
    ('localization_nl.cpp', 'Dutch'),
    ('localization_no.cpp', 'Norwegian'),
    ('localization_pl.cpp', 'Polish'),
    ('localization_pt.cpp', 'Portuguese'),
    ('localization_ro.cpp', 'Romanian'),
    ('localization_ru.cpp', 'Russian'),
    ('localization_sv.cpp', 'Swedish'),
    ('localization_zh.cpp', 'Chinese')
]

for lang_file, lang_name in languages:
    print(f'\nProcessing {lang_name} ({lang_file})...')
    
    # Read language file
    with open(lang_file, 'r', encoding='utf-8') as f:
        lang_content = f.read()
    
    # Extract all key-value pairs from language file
    lang_entries = re.findall(r'{\"([^\"]+)\",\s*\"([^\"]+)\"}', lang_content)
    print(f'  Current entries: {len(lang_entries)}')
    
    # Create a dictionary from language entries (first occurrence wins)
    lang_dict = {}
    for key, value in lang_entries:
        if key not in lang_dict:
            lang_dict[key] = value
    
    print(f'  Unique entries: {len(lang_dict)}')
    
    # Build new content based on German keys
    new_lines = []
    new_lines.append('// ---------------------------------------------------------------------------')
    new_lines.append(f'// {lang_file} – {lang_name} translations for Farming Tracker')
    new_lines.append('// ---------------------------------------------------------------------------')
    new_lines.append('')
    new_lines.append('#include "localization.h"')
    new_lines.append('#include <unordered_map>')
    new_lines.append('')
    new_lines.append('namespace Localization')
    new_lines.append('{')
    new_lines.append(f'    const std::unordered_map<std::string, const char*> Get{lang_name}Translations()')
    new_lines.append('    {')
    new_lines.append('        static const std::unordered_map<std::string, const char*> translations = {')
    
    # Add entries in the same order as German file
    for key, de_value in de_entries:
        if key in lang_dict:
            en_value = lang_dict[key]
        else:
            en_value = de_value  # Use German translation as fallback
            print(f'  Missing {lang_name} translation for key: {key}')
        new_lines.append(f'            {{\"{key}\", \"{en_value}\"}},')
    
    new_lines.append('        };')
    new_lines.append('        return translations;')
    new_lines.append('    }')
    new_lines.append('}')
    
    # Write new language file
    with open(lang_file, 'w', encoding='utf-8') as f:
        f.write('\n'.join(new_lines))
    
    print(f'  New file created with {len(de_entries)} entries')

print('\nDone!')
