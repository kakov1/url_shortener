import pytest

from helpers import wait_until_ready


@pytest.fixture(scope="session", autouse=True)
def ensure_service_ready():
    wait_until_ready()
