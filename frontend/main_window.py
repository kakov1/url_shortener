from __future__ import annotations

from typing import Any

from PySide6.QtCore import Qt
from PySide6.QtGui import QGuiApplication
from PySide6.QtWidgets import (
    QFormLayout,
    QGroupBox,
    QHBoxLayout,
    QHeaderView,
    QLabel,
    QLineEdit,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QStatusBar,
    QTableWidget,
    QTableWidgetItem,
    QTabWidget,
    QTextEdit,
    QVBoxLayout,
    QWidget,
)

from api_client import ApiClient, ApiError


class MainWindow(QMainWindow):
    def __init__(self, api_client: ApiClient) -> None:
        super().__init__()
        self.api_client = api_client

        self.setWindowTitle("URL Shortener GUI")
        self.resize(900, 600)

        self._build_ui()

    def _build_ui(self) -> None:
        tabs = QTabWidget()

        tabs.addTab(self._build_shorten_tab(), "Create URL")
        tabs.addTab(self._build_user_urls_tab(), "User URLs")
        tabs.addTab(self._build_server_tab(), "Server")

        self.setCentralWidget(tabs)
        self.setStatusBar(QStatusBar())
        self.statusBar().showMessage("Ready")

    def _build_shorten_tab(self) -> QWidget:
        page = QWidget()
        layout = QVBoxLayout(page)

        form_box = QGroupBox("Create short URL")
        form_layout = QFormLayout(form_box)

        self.username_input = QLineEdit()
        self.username_input.setPlaceholderText("optional username")

        self.original_url_input = QLineEdit()
        self.original_url_input.setPlaceholderText("https://example.com/very/long/url")

        self.shorten_button = QPushButton("Shorten")
        self.shorten_button.clicked.connect(self.on_shorten_clicked)

        form_layout.addRow("Username:", self.username_input)
        form_layout.addRow("Original URL:", self.original_url_input)
        form_layout.addRow("", self.shorten_button)

        result_box = QGroupBox("Result")
        result_layout = QVBoxLayout(result_box)

        self.result_text = QTextEdit()
        self.result_text.setReadOnly(True)

        self.copy_button = QPushButton("Copy short URL")
        self.copy_button.clicked.connect(self.on_copy_clicked)

        result_layout.addWidget(self.result_text)
        result_layout.addWidget(self.copy_button, alignment=Qt.AlignmentFlag.AlignRight)

        layout.addWidget(form_box)
        layout.addWidget(result_box)

        return page

    def _build_user_urls_tab(self) -> QWidget:
        page = QWidget()
        layout = QVBoxLayout(page)

        top_box = QGroupBox("Load user's URLs")
        top_layout = QFormLayout(top_box)

        self.user_id_input = QLineEdit()
        self.user_id_input.setPlaceholderText("enter user id")

        self.load_urls_button = QPushButton("Load URLs")
        self.load_urls_button.clicked.connect(self.on_load_urls_clicked)

        top_layout.addRow("User ID:", self.user_id_input)
        top_layout.addRow("", self.load_urls_button)

        self.urls_table = QTableWidget(0, 5)
        self.urls_table.setHorizontalHeaderLabels(
            ["ID", "Original URL", "Short Key", "Short URL", "Created At"]
        )
        self.urls_table.horizontalHeader().setSectionResizeMode(
            0, QHeaderView.ResizeMode.ResizeToContents
        )
        self.urls_table.horizontalHeader().setSectionResizeMode(
            1, QHeaderView.ResizeMode.Stretch
        )
        self.urls_table.horizontalHeader().setSectionResizeMode(
            2, QHeaderView.ResizeMode.ResizeToContents
        )
        self.urls_table.horizontalHeader().setSectionResizeMode(
            3, QHeaderView.ResizeMode.ResizeToContents
        )
        self.urls_table.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)

        layout.addWidget(top_box)
        layout.addWidget(self.urls_table)

        return page

    def _build_server_tab(self) -> QWidget:
        page = QWidget()
        layout = QVBoxLayout(page)

        config_box = QGroupBox("Backend")
        config_layout = QFormLayout(config_box)

        self.base_url_input = QLineEdit(self.api_client.base_url)

        self.apply_base_url_button = QPushButton("Apply base URL")
        self.apply_base_url_button.clicked.connect(self.on_apply_base_url_clicked)

        config_layout.addRow("Base URL:", self.base_url_input)
        config_layout.addRow("", self.apply_base_url_button)

        health_box = QGroupBox("Health")
        health_layout = QVBoxLayout(health_box)

        button_row = QHBoxLayout()
        self.health_button = QPushButton("Check health")
        self.health_button.clicked.connect(self.on_health_clicked)

        button_row.addWidget(self.health_button)
        button_row.addStretch()

        self.health_result = QTextEdit()
        self.health_result.setReadOnly(True)

        health_layout.addLayout(button_row)
        health_layout.addWidget(self.health_result)

        layout.addWidget(config_box)
        layout.addWidget(health_box)

        return page

    def on_apply_base_url_clicked(self) -> None:
        base_url = self.base_url_input.text().strip()
        if not base_url:
            self._show_error("Base URL cannot be empty.")
            return

        self.api_client.base_url = base_url
        self.statusBar().showMessage(f"Base URL set to {base_url}")

    def on_health_clicked(self) -> None:
        try:
            data = self.api_client.health()
            self.health_result.setPlainText(str(data))
            self.statusBar().showMessage("Server is available")
        except Exception as exc:
            self.health_result.setPlainText(str(exc))
            self._show_error(f"Health check failed:\n{exc}")

    def on_shorten_clicked(self) -> None:
        username = self.username_input.text().strip()
        original_url = self.original_url_input.text().strip()

        if not original_url:
            self._show_error("Original URL is required.")
            return

        try:
            user_id = None

            if username:
                user = self.api_client.create_user(username)
                user_id = user["id"]

            data = self.api_client.shorten_url(
                original_url=original_url,
                user_id=user_id,
            )

            self.result_text.setPlainText(self._format_result(data))
            self.statusBar().showMessage("Short URL created")

        except ApiError as exc:
            self._show_error(f"API error:\n{exc}")
        except Exception as exc:
            self._show_error(f"Request failed:\n{exc}")

    def on_load_urls_clicked(self) -> None:
        user_id_raw = self.user_id_input.text().strip()
        if not user_id_raw:
            self._show_error("User ID is required.")
            return

        try:
            user_id = int(user_id_raw)
        except ValueError:
            self._show_error("User ID must be an integer.")
            return

        try:
            data = self.api_client.get_user_urls(user_id)

            if not isinstance(data, dict):
                self._show_error(f"Unexpected response format from backend:\n{data}")
                return

            rows = data.get("urls")
            if not isinstance(rows, list):
                self._show_error(f"Unexpected response format from backend:\n{data}")
                return

            self._fill_urls_table(rows)
            backend_user_id = data.get("user_id", user_id)
            self.statusBar().showMessage(
                f"Loaded {len(rows)} URL(s) for user {backend_user_id}"
            )

        except ApiError as exc:
            self._show_error(f"API error:\n{exc}")
        except Exception as exc:
            self._show_error(f"Request failed:\n{exc}")

    def on_copy_clicked(self) -> None:
        text = self.result_text.toPlainText()
        if not text:
            self._show_error("There is nothing to copy yet.")
            return

        short_url = self._extract_short_url(text)
        if not short_url:
            self._show_error("Short URL not found in result.")
            return

        QGuiApplication.clipboard().setText(short_url)
        self.statusBar().showMessage("Short URL copied to clipboard")

    def _fill_urls_table(self, rows: list[dict[str, Any]]) -> None:
        self.urls_table.setRowCount(0)

        for row_idx, row in enumerate(rows):
            self.urls_table.insertRow(row_idx)

            values = [
                str(row.get("id", "")),
                str(row.get("original_url", row.get("url", ""))),
                str(row.get("short_key", "")),
                str(row.get("short_url", "")),
                str(row.get("created_at", "")),
            ]

            for col_idx, value in enumerate(values):
                self.urls_table.setItem(row_idx, col_idx, QTableWidgetItem(value))

    def _format_result(self, data: dict[str, Any]) -> str:
        lines = []
        for key in ("id", "original_url", "short_key", "short_url", "created_at", "user_id"):
            if key in data:
                lines.append(f"{key}: {data[key]}")
        if not lines:
            return str(data)
        return "\n".join(lines)

    def _extract_short_url(self, text: str) -> str | None:
        for line in text.splitlines():
            if line.startswith("short_url:"):
                return line.split(":", 1)[1].strip()
        return None

    def _show_error(self, message: str) -> None:
        QMessageBox.critical(self, "Error", message)
        self.statusBar().showMessage("Error")
