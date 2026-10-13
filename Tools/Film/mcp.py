"""UE 에디터 MCP(127.0.0.1:8000/mcp) 한 번 부르기 — /cl9film 진행기가 StartPIE·StopPIE 에 쓴다.
쓰기(모듈): from mcp import call; call('EditorToolset.EditorAppToolset', 'StartPIE', {'options': {'playMode': 'PlayMode_InEditorFloating'}})
"""
import json
import urllib.request

URL = 'http://127.0.0.1:8000/mcp'


def _post(payload, session=None):
    req = urllib.request.Request(URL, data=json.dumps(payload).encode('utf-8'), method='POST')
    req.add_header('Content-Type', 'application/json')
    req.add_header('Accept', 'application/json, text/event-stream')
    if session:
        req.add_header('Mcp-Session-Id', session)
    with urllib.request.urlopen(req, timeout=600) as resp:
        sid = resp.headers.get('Mcp-Session-Id') or session
        body = resp.read().decode('utf-8', 'replace')
    if body.lstrip().startswith('event:') or '\ndata:' in body or body.startswith('data:'):
        lines = [l[5:].strip() for l in body.splitlines() if l.startswith('data:')]
        body = lines[-1] if lines else ''
    return sid, (json.loads(body) if body.strip() else None)


def call(toolset, tool, args):
    """도구 결과 글(문자열)을 돌려준다"""
    sid, _ = _post({'jsonrpc': '2.0', 'id': 1, 'method': 'initialize',
                    'params': {'protocolVersion': '2025-03-26', 'capabilities': {}, 'clientInfo': {'name': 'cl9film', 'version': '1'}}})
    _post({'jsonrpc': '2.0', 'method': 'notifications/initialized'}, sid)
    _, res = _post({'jsonrpc': '2.0', 'id': 2, 'method': 'tools/call',
                    'params': {'name': 'call_tool', 'arguments': {'toolset_name': toolset, 'tool_name': tool, 'arguments': args}}}, sid)
    try:
        return '\n'.join(c.get('text', '') for c in res['result']['content'] if isinstance(c, dict))
    except Exception:
        return json.dumps(res, ensure_ascii=False)
