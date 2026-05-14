FROM node:20-alpine AS node-builder

Install build deps for C++ if needed (alpine build tools)
RUN apk add --no-cache build-base

WORKDIR /app

1) Build frontend
COPY frontend/package*.json frontend/
WORKDIR /app/frontend
RUN npm ci
COPY frontend/ .
RUN npm run build

2) Install proxy dependencies
WORKDIR /app/proxy
COPY proxy/package*.json proxy/
RUN npm ci --production
COPY proxy/ .

3) Build backend (assumes simple make/CMake or source files that compile to ./server)
WORKDIR /app/backend

Copy backend sources
COPY backend/ .

Example simple build: adjust to your actual build steps
If you already commit built binary, skip build steps and just COPY the binary.
RUN if [ -f Makefile]; then make; fi || true

If your project uses a different build system, replace the command above.
If no build step (already have binary named server), do nothing.
Final runtime image (smaller)
FROM node:20-alpine

Install tini for proper signal handling
RUN apk add --no-cache tini

WORKDIR /app

Copy built frontend
COPY --from=node-builder /app/frontend/dist ./frontend/dist

Copy proxy (node app)
COPY --from=node-builder /app/proxy ./proxy

Copy backend binary or sources (ensure 'server' is executable)
COPY --from=node-builder /app/backend ./backend

Ensure server is executable
RUN if [ -f ./backend/server]; then chmod +x ./backend/server; fi

Expose port that Render will set in $PORT; we document 10000 in app but don't hardcode here
EXPOSE 10000

Copy start script
COPY start.sh /app/start.sh
RUN chmod +x /app/start.sh

Use tini as init and run start.sh
ENTRYPOINT ["/sbin/tini", "--"]
CMD ["/app/start.sh"]

