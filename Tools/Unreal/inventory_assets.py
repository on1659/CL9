"""Read live CL9 definition rows using the editor's reflected property tools."""
import json
from pathlib import Path
import mcp_client as mcp

def call(tool, **arguments):
    result = mcp.rpc('tools/call', {'name':'call_tool','arguments':{
        'toolset_name':'editor_toolset.toolsets.object.ObjectTools',
        'tool_name':tool,'arguments':arguments}})
    if result.get('error') or result.get('result', {}).get('isError'):
        raise RuntimeError(result)
    value = json.loads(result['result']['content'][0]['text'])['returnValue']
    return json.loads(value) if isinstance(value,str) else value

mcp.rpc('initialize', {'protocolVersion':'2025-11-25','capabilities':{},
    'clientInfo':{'name':'CL9Inventory','version':'1'}})
mcp.rpc('notifications/initialized', request_id=None)
inventory = {}
for category in ['Item','Monster','Character','Piece','Module','Site','Equipment']:
    path = f'/Game/Data/DA_{category}Table.DA_{category}Table'
    rows = call('get_properties',instance={'refPath':path},properties=['rows'])['rows']
    inventory[category] = []
    for row in rows:
        schema = call('list_properties',instance=row)
        values = call('get_properties',instance=row,properties=list(schema))
        inventory[category].append({'path':row['refPath'],'values':values})
    print(category, len(rows), flush=True)
    Path('Saved/asset-live-inventory.json').write_text(json.dumps(inventory,ensure_ascii=False,indent=2),encoding='utf-8')
