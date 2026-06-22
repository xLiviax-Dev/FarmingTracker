import re
import os
from collections import defaultdict

def extract_keys_from_file(filepath):
    """Extract all keys from a localization file."""
    keys = set()
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
        # Find all patterns like {"key", "value"}
        matches = re.findall(r'\{"([^"]+)",\s*"[^"]*"\}', content)
        keys.update(matches)
    return keys

def main():
    base_dir = r'd:\Gw2 Projekte\FarmingTracker\src'
    
    # Language files
    languages = {
        'en': 'localization_en.cpp',
        'de': 'localization_de.cpp',
        'fr': 'localization_fr.cpp',
        'es': 'localization_es.cpp',
        'zh': 'localization_zh.cpp',
        'cs': 'localization_cs.cpp',
        'it': 'localization_it.cpp',
        'pl': 'localization_pl.cpp',
        'pt': 'localization_pt.cpp',
        'ru': 'localization_ru.cpp',
        'da': 'localization_da.cpp',
        'el': 'localization_el.cpp',
        'fi': 'localization_fi.cpp',
        'hu': 'localization_hu.cpp',
        'nl': 'localization_nl.cpp',
        'no': 'localization_no.cpp',
        'ro': 'localization_ro.cpp',
        'sv': 'localization_sv.cpp',
    }
    
    # Extract keys from all files
    all_keys = {}
    for lang_code, filename in languages.items():
        filepath = os.path.join(base_dir, filename)
        if os.path.exists(filepath):
            keys = extract_keys_from_file(filepath)
            all_keys[lang_code] = keys
            print(f"{lang_code}: {len(keys)} keys")
        else:
            print(f"{lang_code}: FILE NOT FOUND")
            all_keys[lang_code] = set()
    
    # Use English as reference
    reference_keys = all_keys.get('en', set())
    print(f"\nReference (English): {len(reference_keys)} keys\n")
    
    # Compare each language with English
    missing_keys = defaultdict(list)
    extra_keys = defaultdict(list)
    
    for lang_code, keys in all_keys.items():
        if lang_code == 'en':
            continue
        
        missing = reference_keys - keys
        extra = keys - reference_keys
        
        if missing:
            missing_keys[lang_code] = sorted(list(missing))
        if extra:
            extra_keys[lang_code] = sorted(list(extra))
    
    # Print results
    print("=" * 60)
    print("MISSING KEYS (compared to English):")
    print("=" * 60)
    
    for lang_code in sorted(missing_keys.keys()):
        count = len(missing_keys[lang_code])
        print(f"\n{lang_code}: {count} missing keys")
        for key in missing_keys[lang_code]:  # Show all
            print(f"  - {key}")
    
    print("\n" + "=" * 60)
    print("EXTRA KEYS (not in English):")
    print("=" * 60)
    
    for lang_code in sorted(extra_keys.keys()):
        count = len(extra_keys[lang_code])
        print(f"\n{lang_code}: {count} extra keys")
        for key in extra_keys[lang_code][:10]:  # Show first 10
            print(f"  - {key}")
        if count > 10:
            print(f"  ... and {count - 10} more")
    
    # Summary
    print("\n" + "=" * 60)
    print("SUMMARY:")
    print("=" * 60)
    
    complete_langs = []
    incomplete_langs = []
    
    for lang_code, keys in all_keys.items():
        if lang_code == 'en':
            continue
        
        missing = reference_keys - keys
        if not missing:
            complete_langs.append(lang_code)
        else:
            incomplete_langs.append((lang_code, len(missing)))
    
    if complete_langs:
        print(f"\n[OK] Complete translations ({len(complete_langs)}):")
        for lang in sorted(complete_langs):
            print(f"  - {lang}")
    
    if incomplete_langs:
        print(f"\n[X] Incomplete translations ({len(incomplete_langs)}):")
        for lang, count in sorted(incomplete_langs, key=lambda x: x[1], reverse=True):
            print(f"  - {lang}: {count} missing keys")

if __name__ == "__main__":
    main()
