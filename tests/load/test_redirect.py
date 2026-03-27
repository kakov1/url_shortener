import time

from helpers import (
    shorten_url,
    resolve_short_key,
    run_parallel,
    print_report,
)


class TestRedirectLoad:
    def test_redirect_heavy_load(self):
        create_response = shorten_url("https://example.com/load/redirect-target")
        assert create_response.status_code == 201

        short_key = create_response.json()["short_key"]

        total_requests = 5000
        workers = 100

        def task(_):
            start = time.perf_counter()
            response = resolve_short_key(short_key)
            elapsed = time.perf_counter() - start
            return elapsed, response.status_code

        stats = run_parallel(total_requests, workers, task)
        print_report("redirect_heavy_load", stats)

        assert stats["failure_count"] == 0
        assert stats["success_rate"] >= 0.99
        assert stats["p95_latency_ms"] < 500
