set -ex

echo "0-Clean"
cd `dirname $0`
docker compose -f docker-compose-web.dev.yml down

echo "2-Start"
docker compose -f docker-compose-web.dev.yml up







