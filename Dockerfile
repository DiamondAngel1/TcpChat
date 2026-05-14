FROM node:20-alpine AS node-builder
RUN apk add --no-cache build-base

WORKDIR /app
COPY frontend/package*.json frontend/
WORKDIR /app/frontend
RUN npm ci
COPY frontend/ .
RUN npm run build

WORKDIR /app/proxy
COPY proxy/package*.json proxy/
RUN npm ci --production
COPY proxy/ .
WORKDIR /app/backend
COPY backend/ .

RUN if [ -f Makefile]; then make; fi || true
FROM node:20-alpine
RUN apk add --no-cache tini

WORKDIR /app
COPY --from=node-builder /app/frontend/dist ./frontend/dist
COPY --from=node-builder /app/proxy ./proxy
COPY --from=node-builder /app/backend ./backend

Ensure server is executable
RUN if [ -f ./backend/server]; then chmod +x ./backend/server; 
EXPOSE 10000
COPY start.sh /app/start.sh
RUN chmod +x /app/start.sh
ENTRYPOINT ["/sbin/tini", "--"]
CMD ["/app/start.sh"]

