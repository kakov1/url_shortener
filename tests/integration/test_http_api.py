import os
import time
import unittest

import requests


BASE_URL = os.getenv("APP_BASE_URL", "http://localhost:8080")


class UrlShortenerApiTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.wait_until_service_is_ready()

    @classmethod
    def wait_until_service_is_ready(cls, timeout_seconds: int = 30) -> None:
        deadline = time.time() + timeout_seconds
        last_error = None

        while time.time() < deadline:
            try:
                response = requests.get(f"{BASE_URL}/health", timeout=2)
                if response.status_code == 200:
                    return
            except requests.RequestException as exc:
                last_error = exc

            time.sleep(1)

        raise RuntimeError(f"service did not become ready in time: {last_error}")

    def test_health(self):
        response = requests.get(f"{BASE_URL}/health", timeout=5)

        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.headers["Content-Type"].split(";")[0], "application/json")
        self.assertEqual(response.json()["status"], "ok")

    def test_create_user(self):
        response = requests.post(
            f"{BASE_URL}/users",
            json={"username": "alice_integration_1"},
            timeout=5,
        )

        self.assertEqual(response.status_code, 201)
        body = response.json()

        self.assertIn("id", body)
        self.assertGreater(body["id"], 0)
        self.assertEqual(body["username"], "alice_integration_1")

    def test_create_user_with_empty_username_returns_400(self):
        response = requests.post(
            f"{BASE_URL}/users",
            json={"username": ""},
            timeout=5,
        )

        self.assertEqual(response.status_code, 400)
        self.assertIn("error", response.json())

    def test_shorten_public_url(self):
        response = requests.post(
            f"{BASE_URL}/shorten",
            json={"url": "https://example.com/public-page"},
            timeout=5,
        )

        self.assertEqual(response.status_code, 201)
        body = response.json()

        self.assertIn("id", body)
        self.assertEqual(body["original_url"], "https://example.com/public-page")
        self.assertIn("short_key", body)
        self.assertTrue(body["short_key"])
        self.assertIn("short_url", body)
        self.assertIsNone(body["user_id"])

    def test_same_public_url_returns_same_record(self):
        first = requests.post(
            f"{BASE_URL}/shorten",
            json={"url": "https://example.com/shared-public"},
            timeout=5,
        )
        second = requests.post(
            f"{BASE_URL}/shorten",
            json={"url": "https://example.com/shared-public"},
            timeout=5,
        )

        self.assertEqual(first.status_code, 201)
        self.assertEqual(second.status_code, 201)

        first_body = first.json()
        second_body = second.json()

        self.assertEqual(first_body["id"], second_body["id"])
        self.assertEqual(first_body["short_key"], second_body["short_key"])

    def test_shorten_url_with_unknown_user_returns_404(self):
        response = requests.post(
            f"{BASE_URL}/shorten",
            json={"url": "https://example.com/private-page", "user_id": 999999},
            timeout=5,
        )

        self.assertEqual(response.status_code, 404)
        self.assertIn("error", response.json())

    def test_shorten_user_url_and_get_user_urls(self):
        create_user_response = requests.post(
            f"{BASE_URL}/users",
            json={"username": "bob_integration_1"},
            timeout=5,
        )
        self.assertEqual(create_user_response.status_code, 201)
        user_id = create_user_response.json()["id"]

        shorten_response = requests.post(
            f"{BASE_URL}/shorten",
            json={
                "url": "https://example.com/bob-page",
                "user_id": user_id,
            },
            timeout=5,
        )
        self.assertEqual(shorten_response.status_code, 201)
        shortened = shorten_response.json()

        self.assertEqual(shortened["user_id"], user_id)
        self.assertEqual(shortened["original_url"], "https://example.com/bob-page")

        list_response = requests.get(f"{BASE_URL}/users/{user_id}/urls", timeout=5)
        self.assertEqual(list_response.status_code, 200)

        listed = list_response.json()
        self.assertEqual(listed["user_id"], user_id)
        self.assertIn("urls", listed)
        self.assertGreaterEqual(len(listed["urls"]), 1)

        found = any(item["id"] == shortened["id"] for item in listed["urls"])
        self.assertTrue(found)

    def test_redirect_by_short_key(self):
        shorten_response = requests.post(
            f"{BASE_URL}/shorten",
            json={"url": "https://example.com/redirect-target"},
            timeout=5,
        )
        self.assertEqual(shorten_response.status_code, 201)

        short_key = shorten_response.json()["short_key"]

        redirect_response = requests.get(
            f"{BASE_URL}/{short_key}",
            allow_redirects=False,
            timeout=5,
        )

        self.assertEqual(redirect_response.status_code, 302)
        self.assertEqual(
            redirect_response.headers["Location"],
            "https://example.com/redirect-target",
        )

    def test_unknown_short_key_returns_404(self):
        response = requests.get(
            f"{BASE_URL}/definitely_unknown_short_key",
            allow_redirects=False,
            timeout=5,
        )

        self.assertEqual(response.status_code, 404)

    def test_invalid_json_returns_400(self):
        response = requests.post(
            f"{BASE_URL}/shorten",
            data='{"url":',
            headers={"Content-Type": "application/json"},
            timeout=5,
        )

        self.assertEqual(response.status_code, 400)
        self.assertIn("error", response.json())

    def test_invalid_url_scheme_returns_400(self):
        response = requests.post(
            f"{BASE_URL}/shorten",
            json={"url": "ftp://example.com/file"},
            timeout=5,
        )

        self.assertEqual(response.status_code, 400)
        self.assertIn("error", response.json())


if __name__ == "__main__":
    unittest.main()
    