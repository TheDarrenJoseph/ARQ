#[cfg(test)]
mod text_number_input {
    use crate::widget::stateful::number_widget::{build_number_input, build_number_input_with_value, NumberInputState};
    use crate::widget::{Focusable, StatefulWidgetType};

    fn assert_for_number_widget<F>(widget_type : StatefulWidgetType, mut assert_callback: F) where F : FnMut(NumberInputState) {
        match widget_type {
            StatefulWidgetType::Number(s ) => {
                assert_callback(s);
            }
            _ => {
                panic!("Widget state type was not Number!")
            }
        }
    }

    #[test]
    fn test_build_number_input() {
        // GIVEN valid inputs
        // WHEN we call to build a number input
        let mut number_input = build_number_input(true,1, "A".to_string(), 1);
        // THEN we expect it to be created without being focused
        assert_eq!(false, number_input.is_focused());
        assert_for_number_widget(number_input,  &|mut state: NumberInputState| {
            assert_eq!(0, state.get_input());
        });
    }

    #[test]
    fn test_build_number_input_with_value() {
        // GIVEN valid inputs
        // WHEN we call to build a number input
        let mut number_input = build_number_input_with_value(true, 100,1, "A".to_string(), 1);
        // THEN we expect it to be created without being focused
        assert_eq!(false, number_input.is_focused());
        assert_for_number_widget(number_input,  &|mut state: NumberInputState| {
            assert_eq!(100, state.get_input());
        });
    }

    #[test]
    fn test_number_input_increment() {
        // GIVEN a number input that's currently set to 99 (1 off the 100 max input)
        let mut number_input = build_number_input_with_value(true, 99,1, "A".to_string(), 1);
        assert_eq!(false, number_input.is_focused());
        assert_for_number_widget(number_input,  &|mut state: NumberInputState| {
            assert_eq!(99, state.get_input());
            // WHEN we call to increment twice
            state.increment();
            assert_eq!(100, state.get_input());
            state.increment();
            // THEN we expect the value to remain at the maximum allowed
            assert_eq!(100, state.get_input());
        });
    }

    #[test]
    fn test_number_input_decrement() {
        // GIVEN a number input that's currently set to 1 (1 off the 0 min input)
        let mut number_input = build_number_input_with_value(true, 1,1, "A".to_string(), 1);
        assert_eq!(false, number_input.is_focused());
        assert_for_number_widget(number_input,  &|mut state: NumberInputState| {
            assert_eq!(1, state.get_input());
            // WHEN we call to decrement twice
            state.decrement();
            assert_eq!(0, state.get_input());
            state.decrement();
            // THEN we expect the value to remain at the minimum allowed
            assert_eq!(0, state.get_input());
        });
    }
}
