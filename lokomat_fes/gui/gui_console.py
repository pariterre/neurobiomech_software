from typing import override
from .gui_generic import GuiGeneric


class GuiConsole(GuiGeneric):
    """Console GUI."""

    @override
    def exec(self):
        """Start the GUI."""
        print("Starting console GUI.")
        print("Press Ctrl+C to exit.")
        while True:
            try:
                pass
            except KeyboardInterrupt:
                print("Exiting console GUI.")
                break
