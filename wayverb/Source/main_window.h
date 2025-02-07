#pragma once

#include "main_model.h"
#include "MainContentComponent.h"

class main_window final : public DocumentWindow,
                          public ApplicationCommandTarget {
public:
    //  load with a custom config too
    main_window(ApplicationCommandTarget& next, String name, std::string fname);
    ~main_window() noexcept;

    /// Returns true if the object is ready to be deleted.
    bool prepare_to_close();
    void closeButtonPressed() override;

    // void show_help();

    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

    void save();
    void save_as();

    using wants_to_close = util::event<main_window&>;
    wants_to_close::connection connect_wants_to_close(
            wants_to_close::callback_type callback);

    //  TODO closeButtonPressed should notify the app that the window needs to
    //  close. It should *not* take matters into its own hands.

private:
    std::optional<std::string> browse_for_file_to_save();

    ApplicationCommandTarget& next_command_target_;

    main_model model_;

    MainContentComponent content_component_;

    main_model::encountered_error::scoped_connection encountered_error_connection_;
    main_model::begun::scoped_connection begun_connection_;
    main_model::finished::scoped_connection finished_connection_;

    wants_to_close wants_to_close_;
};
