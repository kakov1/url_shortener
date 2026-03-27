import time
import uuid

from helpers import shorten_url, resolve_short_key, run_parallel, print_report


class TestMixedLoad:
    def test_mixed_read_write_load(self):
        seed_urls = []
        for i in range(50):
            response = shorten_url(f"https://example.com/mixed/seed/{i}")
            assert response.status_code == 201
            seed_urls.append(response.json()["short_key"])

        total_requests = 4000
        workers = 100

        def task(i):
            start = time.perf_counter()

            if i % 4 == 0:
                response = shorten_url(
                    f"https://example.com/mixed/write/{i}-{uuid.uuid4().hex}"
                )
            else:
                short_key = seed_urls[i % len(seed_urls)]
                response = resolve_short_key(short_key)

            elapsed = time.perf_counter() - start
            return elapsed, response.status_code

        stats = run_parallel(total_requests, workers, task)
        print_report("mixed_read_write_load", stats)

        assert stats["failure_count"] == 0
        assert stats["success_rate"] >= 0.98
        assert stats["p95_latency_ms"] < 800