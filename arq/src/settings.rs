use crate::ui::bindings::action_bindings::build_default_action_keybindings;
use crate::ui::bindings::input_bindings::{AllKeyBindings, CommandSpecificKeyBindings};
use crate::ui::bindings::inventory_bindings::InventoryKeyBindings;
use crate::ui::bindings::look_bindings::build_default_look_keybindings;
use crate::ui::bindings::open_bindings::build_default_open_keybindings;
use crate::ui::resolution::Resolution;
use crate::widget::stateful::dropdown_widget::{get_resolution_dropdown_options, DropdownOption, DropdownSetting};
use rand::distr::Alphanumeric;
use rand::Rng;
use std::fs;

pub const SETTING_FOG_OF_WAR : &str = "Fog of War";
pub const SETTING_RNG_SEED : &str = "Map RNG Seed";
pub const SETTING_BG_MUSIC : &str = "Background music";
pub const SETTING_RESOLUTION : &str = "Resolution";

pub const RESOURCE_SETTINGS_FILE: &str = "resources/settings.json";

pub struct Setting<T> {
    pub name : String,
    pub value : T
}

pub struct Settings {
    pub bool_settings : Vec<Setting<bool>>,
    pub u32_settings : Vec<Setting<u32>>,
    pub string_settings : Vec<Setting<String>>,
    pub dropdown_settings : Vec<Setting<DropdownSetting<DropdownOption<Resolution>>>>,
    pub key_bindings: AllKeyBindings
}

impl Settings {
    pub fn find_string_setting_value(&self, name : String) -> Option<String> {
        let setting = self.string_settings.iter().find(|x| x.name == name);
        if let Some(s) = setting {
            return Some(s.value.clone());
        }
        None
    }

    pub fn find_bool_setting_value(&self, name : String) -> Option<bool> {
        let setting = self.bool_settings.iter().find(|x| x.name == name);
        if let Some(s) = setting {
            return Some(s.value.clone());
        }
        None
    }

    pub fn find_u32_setting_value(&self, name : String) -> Option<u32> {
        let setting = self.u32_settings.iter().find(|x| x.name == name);
        if let Some(s) = setting {
            return Some(s.value.clone());
        }
        None
    }

    pub fn find_dropdown_setting_value(&self, name : String) -> Option<DropdownOption<Resolution>> {
        let setting = self.dropdown_settings.iter().find(|setting| setting.name == name);
        if let Some(s) = setting {
            return Some(s.value.chosen_option.clone());
        }
        None
    }


    /*
    * Either returns the bool value for SETTING_FOG_OF_WAR, or defaults to false
     */
    pub fn is_fog_of_war(&self) -> bool {
        self.find_bool_setting_value(SETTING_FOG_OF_WAR.to_string()).or_else(|| Some(false)).unwrap()
    }

    pub fn get_rng_seed(&self) -> Option<String> {
        // if let Some(seed_override) = GLOBALS.rng_seed_override {
        //     return Some(String::from(seed_override))
        // } else {
            return self.find_string_setting_value(SETTING_RNG_SEED.to_string())
        // }
    }

    /*
    * Either returns the bool value for SETTING_BG_MUSIC, or defaults to 100%
     */
    pub fn get_bg_music_volume(&self) -> u32 {
        self.find_u32_setting_value(SETTING_BG_MUSIC.to_string()).or_else(|| Some(100)).unwrap()
    }

    pub fn get_resolution(&self) -> DropdownOption<Resolution> {
        self.find_dropdown_setting_value(SETTING_RESOLUTION.to_string()).unwrap()
    }
}

pub fn build_default_bindings() -> AllKeyBindings {
    AllKeyBindings {
        action_key_bindings: build_default_action_keybindings(),
        command_specific_key_bindings: CommandSpecificKeyBindings {
            inventory_key_bindings: InventoryKeyBindings { bindings: Default::default() },
            look_key_bindings: build_default_look_keybindings(),
            open_key_bindings: build_default_open_keybindings()
        },
    }
}

pub fn load_settings_file() -> serde_json::Value {
    let settings_raw = fs::read_to_string(RESOURCE_SETTINGS_FILE).unwrap();
    serde_json::from_str(&settings_raw).unwrap()
}

fn get_random_map_seed() -> String {
    // Generate a new random seed
    rand::rng()
        .sample_iter(&Alphanumeric)
        .take(12)
        .map(char::from)
        .collect()
}

fn get_map_seed(settings_json: &serde_json::Value) -> String {
    // INITIAL_MAP_SEED allows setting the map seed ahead of time, useful for debugging
    let initial_map_seed : Option<String> = settings_json.get("INITIAL_MAP_SEED").map(|v| String::from(v.as_str().unwrap()));

    if initial_map_seed.is_some()  {
        initial_map_seed.unwrap()
    } else {
        get_random_map_seed()
    }
}

pub fn build_settings() -> Settings {
    let settings_json = load_settings_file();
    let bg_music_volume_default : u32 = settings_json.get("BG_MUSIC_VOLUME_DEFAULT").unwrap().as_u64().unwrap() as u32;
    let resolution_default : String = String::from(settings_json.get("RESOLUTION_DEFAULT").unwrap().as_str().unwrap());
    let fog_of_war : Setting<bool> = Setting { name: SETTING_FOG_OF_WAR.to_string(), value: false };

    let map_seed_value = get_map_seed(&settings_json);
    let map_seed : Setting<String> = Setting { name: SETTING_RNG_SEED.to_string(), value: map_seed_value };
    let bg_music_volume : Setting<u32> = Setting { name: SETTING_BG_MUSIC.to_string(), value: bg_music_volume_default };

    let resolution_options = get_resolution_dropdown_options();
    let mut default_resolution_option = None;
    if !resolution_default.is_empty() {
        default_resolution_option = resolution_options.iter()
            .find(|opt| opt.display_name.eq(resolution_default.as_str()))
            .map(|opt| opt.clone())
            .take()
    }

    let initial_option = if default_resolution_option.is_some() {
        default_resolution_option.unwrap()
    } else {
         resolution_options.first().unwrap().clone()
    };
    let resolution_dropdown_setting : DropdownSetting<DropdownOption<Resolution>> = DropdownSetting {
        options: resolution_options.clone(),
        chosen_option: initial_option.clone()
    };
    let resolution : Setting<DropdownSetting<DropdownOption<Resolution>>> = Setting { name: SETTING_RESOLUTION.to_string(), value: resolution_dropdown_setting };
    Settings { bool_settings: vec![fog_of_war], string_settings: vec![map_seed], u32_settings: vec![bg_music_volume], dropdown_settings: vec![resolution], key_bindings: build_default_bindings() }
}

pub trait Toggleable {
    fn toggle(&mut self);
}

impl Toggleable for Setting<bool> {
    fn toggle(&mut self) {
        self.value = !self.value;
    }
}
