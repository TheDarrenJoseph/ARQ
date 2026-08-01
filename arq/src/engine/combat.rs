use crate::character::battle::Battle;
use crate::character::equipment::WeaponSlot;

#[derive(Debug, Clone)]
pub enum CombatTurnChoiceEventType {
    ATTACK(WeaponSlot),
    FLEE
}

pub struct Combat {
    pub(crate) battle: Battle
}

#[derive(Clone)]
pub struct CombatResult {
    pub(crate) messages: Vec<String>
}