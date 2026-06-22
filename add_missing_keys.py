import re

# Missing keys with English translations
missing_keys_all = {
    "blacklist_add_item": "Add Item by ID",
    "blacklist_add_item_tooltip": "Add an item to the blacklist by its ID",
    "blacklist_item_id": "Item ID",
    "blacklist_item_id_tooltip": "Enter the item ID to add to the blacklist",
    "column_magic_find": "Magic Find",
    "get_drf_token": "Get your DRF Token",
    "get_gw2_api_key": "Get your GW2 API Key",
    "main_window_font_size": "Main Window Font Size",
    "main_window_font_size_tooltip": "Font size scale for the main window (0.5 = smaller, 1.0 = default, 2.0 = larger)",
    "main_window_hide_title_bar": "Hide Main Window Title Bar",
    "main_window_hide_title_bar_tooltip": "Hide the title bar of the main window",
    "mini_window_best_drop_icon_size": "Drop Icon Size",
    "mini_window_best_drop_icon_size_tooltip": "Size of the best drop icons in the mini window",
    "mini_window_element_order": "Mini Window Element Order",
    "mini_window_element_order_tooltip": "Order of elements in the mini window",
    "mini_window_enable_text_shadow": "Text Outline",
    "mini_window_enable_text_shadow_tooltip": "Add black outline to text for better visibility at high transparency",
    "mini_window_font_size": "Mini Window Font Size",
    "mini_window_font_size_tooltip": "Font size for the mini window text",
    "mini_window_hide_border": "Hide Mini Window Border",
    "mini_window_hide_border_tooltip": "Hide the border of the mini window",
    "mini_window_show_best_drop_icons": "Show Drop Icons",
    "mini_window_show_best_drop_icons_tooltip": "Show icons for best drops in the mini window",
    "mini_window_text_color": "Mini Window Text Color",
    "mini_window_text_color_tooltip": "Text color for the mini window",
    "notification_blacklist": "Blacklist",
    "notification_blacklist_tooltip": "Items excluded from notifications",
    "show_short_icon": "Show Shortcut Icon",
    "show_short_icon_tooltip": "Show the Farming Tracker shortcut icon in the Nexus Quick Access bar",
    "sparkline_color": "Sparkline Color",
    "sparkline_color_tooltip": "Color for the profit sparkline graph",
}

# Additional missing keys for specific languages
additional_keys = {
    "notification_font_size": "Notification Font Size",
    "notification_font_size_tooltip": "Font size scale for notifications (0.5 = smaller, 1.0 = default, 2.0 = larger)",
    "select_log_folder": "Select log folder",
    "tab_overview": "Overview",
}

# Language files with specific additional keys needed
languages = {
    'de': ('localization_de.cpp', ['notification_font_size', 'notification_font_size_tooltip', 'select_log_folder']),
    'fr': ('localization_fr.cpp', []),
    'es': ('localization_es.cpp', []),
    'zh': ('localization_zh.cpp', []),
    'cs': ('localization_cs.cpp', ['notification_font_size', 'notification_font_size_tooltip']),
    'it': ('localization_it.cpp', []),
    'pl': ('localization_pl.cpp', []),
    'pt': ('localization_pt.cpp', []),
    'ru': ('localization_ru.cpp', []),
    'da': ('localization_da.cpp', ['notification_font_size', 'notification_font_size_tooltip']),
    'el': ('localization_el.cpp', ['notification_font_size', 'notification_font_size_tooltip', 'tab_overview']),
    'fi': ('localization_fi.cpp', []),
    'hu': ('localization_hu.cpp', []),
    'nl': ('localization_nl.cpp', []),
    'no': ('localization_no.cpp', []),
    'ro': ('localization_ro.cpp', []),
    'sv': ('localization_sv.cpp', []),
}

base_dir = r'd:\Gw2 Projekte\FarmingTracker\src'

for lang_code, (filename, additional_needed) in languages.items():
    filepath = f"{base_dir}\\{filename}"
    
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Find the last line before the closing bracket
    # Look for the pattern }; at the end
    closing_bracket_pos = content.rfind('};')
    if closing_bracket_pos == -1:
        print(f"ERROR: Could not find closing bracket in {filename}")
        continue
    
    # Find the last comma before the closing bracket
    last_comma_pos = content.rfind(',', 0, closing_bracket_pos)
    if last_comma_pos == -1:
        print(f"ERROR: Could not find last comma in {filename}")
        continue
    
    # Insert the missing keys before the closing bracket
    insert_pos = last_comma_pos + 1
    
    # Build the string to insert - combine all keys and additional keys
    keys_to_add = []
    for key, value in missing_keys_all.items():
        keys_to_add.append(f'            {{"{key}", "{value}"}},')
    
    # Add additional keys if needed
    for key in additional_needed:
        if key in additional_keys:
            keys_to_add.append(f'            {{"{key}", "{additional_keys[key]}"}},')
    
    insert_string = '\n' + '\n'.join(keys_to_add) + '\n'
    
    # Insert the new keys
    new_content = content[:insert_pos] + insert_string + content[insert_pos:]
    
    # Write back to file
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(new_content)
    
    print(f"Added {len(keys_to_add)} keys to {filename}")

print("\nDone!")
