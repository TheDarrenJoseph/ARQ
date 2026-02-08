use termion::event::Key;

pub mod utils;

/*
    Describes a Termion Key Event in a UI friendly way
 */
pub fn describe_key(key: Key) -> String{
    match key {
        Key::Backspace => String::from("Enter"),
        Key::Char(c) => String::from(c),
        _ => format!("{:?}", key),
    }
}

#[cfg(test)]
mod tests {
    use termion::event::Key;

    #[test]
    fn describe_key_char() {
        // GIVEN a random Key::Char(c)
        let key = Key::Char('u');

        // WHEN we call to describe this key
        let description = super::describe_key(key);

        // THEN we expect the description to be a literal 'u' character
        assert_eq!("u", description);
    }

    #[test]
    fn describe_key_escape() {
        // GIVEN a Key::Escape (a key that is not represented with a single character)
        let key = Key::Esc;

        // WHEN we call to describe this key
        let description = super::describe_key(key);

        // THEN we expect the description to be "Esc"
        assert_eq!("Esc", description);
    }

    #[test]
    fn describe_key_enter() {
        // GIVEN a Key::Backspace (enter key)
        let key = Key::Backspace;

        // WHEN we call to describe this key
        let description = super::describe_key(key);

        // THEN we expect the description to be "Enter"
        assert_eq!("Enter", description);
    }
}