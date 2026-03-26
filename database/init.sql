CREATE TABLE IF NOT EXISTS users (
    id BIGSERIAL PRIMARY KEY,
    username VARCHAR(255) NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS urls (
    id BIGSERIAL PRIMARY KEY,
    original_url TEXT NOT NULL,
    short_key VARCHAR(32) NOT NULL UNIQUE,
    user_id BIGINT REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_urls_user_id ON urls(user_id);
CREATE INDEX IF NOT EXISTS idx_urls_original_url ON urls(original_url);

CREATE UNIQUE INDEX IF NOT EXISTS uq_urls_public_original_url
    ON urls(original_url)
    WHERE user_id IS NULL;

CREATE UNIQUE INDEX IF NOT EXISTS uq_urls_user_original_url
    ON urls(user_id, original_url)
    WHERE user_id IS NOT NULL;
    