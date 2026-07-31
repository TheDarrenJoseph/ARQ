#[cfg(test)]
mod test_text_input {
    use crate::widget::stateful::text_widget::{build_text_input, TextInputState};
    use crate::widget::StatefulWidgetType;

    fn assert_for_text_widget<F>(widget_type : StatefulWidgetType, mut callback: F) where F : FnMut(TextInputState) {
        match widget_type {
            StatefulWidgetType::Text(s ) => {
                callback(s);
            }
            _ => {
                panic!("Widget state type was not Text!")
            }
        }
    }

    #[test]
    fn test_text_input_add_char() {
        // GIVEN a text input of 3 characters with no initial input
        let text_input = build_text_input(3, "Input".to_string(),"".to_string(), 1);
        assert_for_text_widget(text_input,  &|mut state: TextInputState| {
            // WHEN we add a character
            state.add_char('A');
            // THEN we expect the widget state input to be "A"
            assert_eq!("A".to_string(),  state.get_input());
        });
    }

    #[test]
    fn test_text_input_add_char_max_input() {
        // GIVEN a text input of 3 characters with no initial input
        let text_input = build_text_input(3, "Input".to_string(), "".to_string(), 1);
        // WHEN we add 4 characters
        assert_for_text_widget(text_input,  &|mut state: TextInputState| {
            state.add_char('A');
            state.add_char('B');
            state.add_char('C');
            state.add_char('D');

            // THEN we expect the widget state input to be "ABC" and to have ignored the extra character
            assert_eq!("ABC".to_string(),  state.get_input());
        });
    }

    #[test]
    fn test_text_input_delete_char() {
        // GIVEN a text input of 3 characters with no initial input
        let text_input = build_text_input(3, "Input".to_string(), "".to_string(), 1);
        assert_for_text_widget(text_input,  &|mut state: TextInputState| {
            // AND we've adjusted it's input to be "A"
            state.set_input("A".to_string());
            // WHEN we call to delete a char
            state.delete_char();
            // THEN we expect the widget state input to be ""
            assert_eq!("".to_string(),  state.get_input());
        });
    }

    #[test]
    fn test_text_input_delete_char_empty_field() {
        // GIVEN a text input of 3 characters with no initial input
        let text_input = build_text_input(3, "Input".to_string(),"".to_string(), 1);
        assert_for_text_widget(text_input,  &|mut state: TextInputState| {
            // WHEN we call to delete a char
            state.delete_char();
            // THEN we expect the widget state input to be ""
            assert_eq!("".to_string(), state.get_input());
        });
    }

    #[test]
    fn test_text_input_delete_char_many() {
        // GIVEN a text input of 3 characters with no initial input
        let text_input = build_text_input(3, "Input".to_string(),"".to_string(), 1);
        assert_for_text_widget(text_input,  &|mut state: TextInputState| {
                // AND we've adjusted it's input to be "ABC"
                state.set_input("ABC".to_string());
                // WHEN we call to delete 2 characters
                state.delete_char();
                state.delete_char();
                // THEN we expect the widget state input to be "A"
                assert_eq!("A".to_string(),  state.get_input());
        });
    }
}