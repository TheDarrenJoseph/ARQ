use std::fmt::{Debug, Display, Formatter, Result};

#[derive(Copy, PartialEq, Eq, Clone, Debug)]
pub enum Attribute {Strength, Health, Agility, Intelligence, Stealth}

impl Display for Attribute {
    fn fmt(&self, f: &mut Formatter) -> Result {
        Debug::fmt(self, f)
    }
}

#[derive(Copy, Clone, Debug)]
#[derive(PartialEq, Eq)]
pub struct AttributeScore {
    pub attribute: Attribute,
    pub score: i8
}

pub struct AttributeScores {
    pub scores : Vec<AttributeScore>
}

impl AttributeScore {
    pub fn default(attribute: Attribute) -> AttributeScore {
        AttributeScore { attribute, score: 0 }
    }

    pub fn new(attribute: Attribute, score: i8) -> AttributeScore {
        AttributeScore { attribute, score }
    }
}

impl AttributeScores {
    pub fn default() -> AttributeScores {
        let mut scores = Vec::new();
        let attributes = all_attributes();
        for attr in attributes {
            scores.push(AttributeScore::default(attr));
        }
        AttributeScores { scores }
    }

    pub fn all_at_value(score: i8) -> AttributeScores {
        let mut scores = Vec::new();
        let attributes = all_attributes();
        for attr in attributes {
            scores.push(AttributeScore::new(attr, score));
        }
        AttributeScores { scores }
    }
}

pub fn build_default_attributes() -> Vec<AttributeScore> {
    let mut scores = Vec::new();
    let attributes = all_attributes();
    for attr in attributes {
        scores.push(AttributeScore { attribute: attr, score: 0 });
    }
    return scores;
}

pub fn all_attributes() -> Vec<Attribute> {
    vec![Attribute::Strength, Attribute::Health, Attribute::Agility, Attribute::Intelligence, Attribute::Stealth]
}