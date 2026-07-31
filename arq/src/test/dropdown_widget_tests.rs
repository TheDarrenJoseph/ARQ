#[cfg(test)]
mod text_dropdown {
    use crate::widget::stateful::dropdown_widget::{build_dropdown, DropdownInputState};
    use crate::widget::StatefulWidgetType;

    fn assert_for_dropdown_widget<F>(widget_type : StatefulWidgetType, mut callback: F) where F : FnMut(DropdownInputState) {
        match widget_type {
            StatefulWidgetType::Dropdown(s ) => {
                callback(s);
            }
            _ => {
                panic!("Widget state type was not text!")
            }
        }
    }

    #[test]
    fn test_dropdown_get_selection() {
        // GIVEN a dropdown with 2 options
        let dropdown = build_dropdown("Test".to_string(), true,vec!["A".to_string(), "B".to_string()]);
        assert_for_dropdown_widget(dropdown,  &|state: DropdownInputState| {
            // WHEN we call to get the initial selection
            // THEN we expect it to be "A"
            assert_eq!("A".to_string(),  state.get_selection());
            // AND be default we don't show options until we've tried to access the dropdown
            assert_eq!(false,  state.is_showing_options());
        });
    }

    #[test]
    fn test_dropdown_toggle_show() {
        // GIVEN a dropdown with 2 options
        let dropdown = build_dropdown("Test".to_string(), true,vec!["A".to_string(), "B".to_string()]);

        assert_for_dropdown_widget(dropdown,  &|mut state: DropdownInputState| {
            // WHEN we call to toggle showing of options
            state.toggle_show();
            // THEN we expect it to be set to true (i.e the dropdown list should be shown)
            assert_eq!(true,  state.is_showing_options());
        });
    }

    #[test]
    fn test_dropdown_toggle_show_multi() {
        // GIVEN a dropdown with 2 options
        let dropdown = build_dropdown("Test".to_string(), true,vec!["A".to_string(), "B".to_string()]);

        assert_for_dropdown_widget(dropdown,  &|mut state: DropdownInputState| {

            // WHEN we call to toggle showing of options twice
            state.toggle_show();
            state.toggle_show();
            // THEN we expect it to be set to false again
            assert_eq!(false,  state.is_showing_options());
        });
    }


    #[test]
    fn test_dropdown_select_next() {
        // GIVEN a dropdown with 2 options
        let dropdown = build_dropdown("Test".to_string(), true,vec!["A".to_string(), "B".to_string()]);
        assert_for_dropdown_widget(dropdown,  &|mut state: DropdownInputState| {
            // WHEN we call to select the next item
            state.select_next();
            // THEN we expect the selection to be "B"
            assert_eq!("B".to_string(),  state.get_selection());
        });
    }

    #[test]
    fn test_dropdown_select_next_end_of_range() {
        // GIVEN a dropdown with 2 options
        let dropdown = build_dropdown("Test".to_string(), true,vec!["A".to_string(), "B".to_string()]);

        assert_for_dropdown_widget(dropdown,  &|mut state: DropdownInputState| {
            // WHEN we call to select the next item twice
            state.select_next();
            state.select_next();
            // THEN we expect the selection to be "B"
            assert_eq!("B".to_string(),  state.get_selection());
        });
    }

    #[test]
    fn test_dropdown_select_previous() {
        // GIVEN a dropdown with 2 options
        let dropdown = build_dropdown("Test".to_string(), true,vec!["A".to_string(), "B".to_string()]);

        assert_for_dropdown_widget(dropdown,  &|mut state: DropdownInputState| {
            // AND we've selected the 2nd option
            state.select_next();
            assert_eq!("B".to_string(),  state.get_selection());
            // WHEN we call to select the next item
            state.select_previous();
            // THEN we expect the selection to be "A" (unchanged)
            assert_eq!("A".to_string(),  state.get_selection());
        });
    }

    #[test]
    fn test_dropdown_select_previous_end_of_range() {
        // GIVEN a dropdown with 2 options
        let dropdown = build_dropdown("Test".to_string(), true,vec!["A".to_string(), "B".to_string()]);

        assert_for_dropdown_widget(dropdown,  &|mut state: DropdownInputState| {
            assert_eq!("A".to_string(),  state.get_selection());
            // WHEN we call to select the next item
            state.select_previous();
            // THEN we expect the selection to be "A" (unchanged)
            assert_eq!("A".to_string(),  state.get_selection());
        });
    }

}