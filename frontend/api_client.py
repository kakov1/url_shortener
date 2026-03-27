from __future__ import annotations

from dataclasses import dataclass
from typing import Any

import requests


class ApiError(Exception):
    pass


@dataclass
class ApiClient:
    base_url: str = "http://localhost:8080"
    timeout: float = 5.0

    def _url(self, path: str) -> str:
        return f"{self.base_url.rstrip('/')}/{path.lstrip('/')}"

    def _handle_response(self, response: requests.Response) -> Any:
        try:
            data = response.json()
        except ValueError:
            data = response.text

        if not response.ok:
            raise ApiError(f"HTTP {response.status_code}: {data}")

        return data

    def health(self) -> dict[str, Any]:
        response = requests.get(self._url("/health"), timeout=self.timeout)
        return self._handle_response(response)

    def create_user(self, username: str) -> dict[str, Any]:
        payload = {"username": username}
        response = requests.post(
            self._url("/users"),
            json=payload,
            timeout=self.timeout,
        )
        return self._handle_response(response)

    def shorten_url(
        self,
        original_url: str,
        username: str | None = None,
        user_id: int | None = None,
    ) -> dict[str, Any]:
        payload: dict[str, Any] = {"url": original_url}

        if username:
            payload["username"] = username
        if user_id is not None:
            payload["user_id"] = user_id

        response = requests.post(
            self._url("/shorten"),
            json=payload,
            timeout=self.timeout,
        )
        return self._handle_response(response)

    def get_user_urls(self, user_id: int) -> dict[str, Any]:
        response = requests.get(
            self._url(f"/users/{user_id}/urls"),
            timeout=self.timeout,
        )
        
        return self._handle_response(response)
