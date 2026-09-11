"""Local UE editor MCP client. Input request JSON from a file; no editor restarts."""
import json
import sys
import urllib.request
from pathlib import Path

URL = 'http://127.0.0.1:8000/mcp'
session = None

def rpc(method, params=None, request_id=1):
    global session
    payload = {'jsonrpc': '2.0', 'method': method}
    if request_id is not None:
        payload['id'] = request_id
    if params is not None:
        payload['params'] = params
    headers = {'Content-Type': 'application/json', 'Accept': 'application/json, text/event-stream'}
    if session:
        headers['Mcp-Session-Id'] = session
    req = urllib.request.Request(URL, json.dumps(payload).encode(), headers)
    with urllib.request.urlopen(req, timeout=120) as response:
        session = response.headers.get('Mcp-Session-Id', session)
        data = response.read().decode('utf-8')
        return json.loads(data) if data.strip() else None

def main():
    rpc('initialize', {'protocolVersion': '2025-11-25', 'capabilities': {},
                      'clientInfo': {'name': 'CL9AssetProduction', 'version': '1.0'}})
    rpc('notifications/initialized', request_id=None)
    request = json.loads(Path(sys.argv[1]).read_text(encoding='utf-8-sig'))
    results = []
    for params in request if isinstance(request, list) else [request]:
        results.append(rpc('tools/call', params))
    output = json.dumps(results, ensure_ascii=False, indent=2)
    if len(sys.argv) > 2:
        Path(sys.argv[2]).write_text(output, encoding='utf-8')
    print(output)

if __name__ == '__main__':
    main()
