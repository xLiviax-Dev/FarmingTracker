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
    
    # Find missing keys
    missing_keys = []
    for key in de_dict:
        if key not in lang_dict:
            missing_keys.append(key)
    
    print(f'  Missing keys: {len(missing_keys)}')
    
    if len(missing_keys) == 0:
        print(f'  {lang_name} is already complete!')
        continue
    
    # Add missing keys with German translations as fallback
    # Find the position before the closing brace
    closing_brace_pos = lang_content.rfind('        };')
    if closing_brace_pos == -1:
        # Try alternative pattern
        closing_brace_pos = lang_content.rfind('};')
    if closing_brace_pos == -1:
        print(f'  ERROR: Could not find closing brace in {lang_file}')
        continue
    
    # Build the new entries to add
    new_entries = []
    for key in missing_keys:
        new_entries.append(f'            {{\"{key}\", \"{de_dict[key]}\"}},')
    
    # Insert the new entries before the closing brace
    new_content = lang_content[:closing_brace_pos] + '\n'.join(new_entries) + '\n' + lang_content[closing_brace_pos:]
    
    # Write the updated file
    with open(lang_file, 'w', encoding='utf-8') as f:
        f.write(new_content)
    
    print(f'  Added {len(missing_keys)} missing keys to {lang_file}')

print('\nDone!')
