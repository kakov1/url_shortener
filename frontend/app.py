from __future__ import annotations

import sys
from PySide6.QtWidgets import QApplication

from api_client import ApiClient
from main_window import MainWindow


def main() -> int:
    app = QApplication(sys.argv)

    api_client = ApiClient(base_url="http://localhost:8080")
    window = MainWindow(api_client)
    window.show()

    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
