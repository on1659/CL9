"""ABP_<종>_A 반응 가지 배선(2026-09-15) — 에디터 MCP `editor_toolset.toolsets.programmatic.ProgrammaticToolset.execute_tool_script` 에
이 파일 내용을 `script` 로 넘긴다(에디터 파이썬이 아니라 MCP 툴 오케스트레이션 스크립트 — `execute_tool` 만 쓸 수 있다). 실행 전 __SPECIES__ 를
Chicken|Goose|Raccoon|Pig|Sheep 로 치환. 하는 일: 변수 StateElapsed·IsPushStun 추가, EventGraph 에 CL9|GetStateElapsed·IsPushStun 세터 삽입,
AnimGraph 의 Catch·Death·WakeTelegraph 플레이어를 평가기(ExplicitTime=StateElapsed)로, HitReact 플레이어를 부울 블렌드(IsPushStun ? PushReact : HitReact)
평가기 쌍으로 교체, 컴파일·저장. 두 번 돌려도 EventGraph 는 건너뛴다(AnimGraph 는 플레이어가 없으면 건너뜀). docs/systems/animation.md "몬스터 원샷".
"""
import json
SP = "__SPECIES__"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools."
BP = "/Game/Monsters/ABP_%s_A.ABP_%s_A" % (SP, SP)
AG = BP + ":AnimGraph"
EG = BP + ":EventGraph"
EVAL = "애니메이션|시퀀스|'%s’평가"          # 애니메이션|시퀀스|'<AN>’평가
BLB = "Animation|Blends|부울로포즈블렌딩"                  # Animation|Blends|부울로포즈블렌딩
VGET = "Variables|디폴트|Get%s"
VSET = "Variables|디폴트|Set%s"


def call(tool, **kw):
    return execute_tool(BT + tool, json.dumps(kw))


def ref(p):
    return {"refPath": p}


def find_all(graph):
    return call("find_nodes", graph=ref(graph), title="")["returnValue"]


def infos(nodes):
    return call("get_node_infos", nodes=nodes)["returnValue"]


def info(node):
    return infos([node])[0]


def pin(ni, name, direction):
    for p in (ni["input_pins"] if direction == "in" else ni["output_pins"]):
        if p["name"] == name:
            return p
    raise RuntimeError("pin %s(%s) not on %s" % (name, direction, ni["node"]["refPath"]))


def create(graph, type_id, x, y):
    return call("create_node", graph=ref(graph), type_id=type_id, pos={"x": int(x), "y": int(y)})["returnValue"]


def connect(out_pin, in_pin):
    call("connect_pins", output_pin=out_pin["pin_id"], input_pin=in_pin["pin_id"])


def brk(out_pin, in_pin_id):
    call("break_pins", output_pin=out_pin["pin_id"], input_pin=in_pin_id)


def setval(p, v):
    call("set_pin_value", pin=p["pin_id"], value=str(v))


def run():
    log = []
    vars_ = call("list_variables", blueprint=ref(BP))["returnValue"]
    if "StateElapsed" not in vars_:
        call("add_variable", blueprint=ref(BP), name="StateElapsed", type_name="float")
    if "IsPushStun" not in vars_:
        call("add_variable", blueprint=ref(BP), name="IsPushStun", type_name="bool")
    log.append("vars ok")

    # ---- EventGraph: SetSpeed.then -> SetStateElapsed -> SetIsPushStun -> (old next) ----
    eg = infos(find_all(EG))
    if not any(n["type_id"].endswith("|SetStateElapsed") for n in eg):
        cast = next(n for n in eg if "CastToCL9Monster" in n["type_id"])
        setspeed = next(n for n in eg if n["type_id"].endswith("|SetSpeed"))
        as_monster = pin(cast, "AsCL9Monster", "out")
        then_p = pin(setspeed, "then", "out")
        old_next = then_p["connected_pins"][0]
        brk(then_p, old_next)
        x, y = setspeed["position"]["x"], setspeed["position"]["y"]
        f1 = info(create(EG, "CL9|GetStateElapsed", x + 20, y + 260))
        connect(as_monster, pin(f1, "self", "in"))
        s1 = info(create(EG, VSET % "StateElapsed", x + 250, y + 200))
        connect(then_p, pin(s1, "execute", "in"))
        connect(pin(f1, "ReturnValue", "out"), pin(s1, "StateElapsed", "in"))
        f2 = info(create(EG, "CL9|IsPushStun", x + 320, y + 360))
        connect(as_monster, pin(f2, "self", "in"))
        s2 = info(create(EG, VSET % "IsPushStun", x + 520, y + 200))
        connect(pin(s1, "then", "out"), pin(s2, "execute", "in"))
        connect(pin(f2, "ReturnValue", "out"), pin(s2, "IsPushStun", "in"))
        call("connect_pins", output_pin=pin(s2, "then", "out")["pin_id"], input_pin=old_next)
        log.append("eventgraph wired")
    else:
        log.append("eventgraph already wired")

    # ---- AnimGraph: one-shot players -> evaluators(StateElapsed); HitReact -> bool blend(IsPushStun) ----
    ag = infos(find_all(AG))
    gse = info(create(AG, VGET % "StateElapsed", 250, 700))
    gse_out = pin(gse, "StateElapsed", "out")
    gps = None

    def player_of(clip):
        for n in ag:
            if ("'AN_%s_%s'" % (SP, clip)) in n["type_id"] and "시퀀스플레이어" in n["type_id"]:
                return n
        return None

    for clip in ("Catch", "Death", "WakeTelegraph"):
        old = player_of(clip)
        if old is None:
            log.append("no player for " + clip)
            continue
        pose = pin(old, "Pose", "out")
        target = pose["connected_pins"][0]
        px, py = old["position"]["x"], old["position"]["y"]
        call("delete_node", node=old["node"])
        ev = info(create(AG, EVAL % ("AN_%s_%s" % (SP, clip)), px, py))
        connect(gse_out, pin(ev, "ExplicitTime", "in"))
        call("connect_pins", output_pin=pin(ev, "Pose", "out")["pin_id"], input_pin=target)
        log.append(clip + " -> evaluator")

    old = player_of("HitReact")
    if old is not None:
        pose = pin(old, "Pose", "out")
        target = pose["connected_pins"][0]
        px, py = old["position"]["x"], old["position"]["y"]
        call("delete_node", node=old["node"])
        hit = info(create(AG, EVAL % ("AN_%s_HitReact" % SP), px - 300, py + 40))
        push = info(create(AG, EVAL % ("AN_%s_PushReact" % SP), px - 300, py - 80))
        connect(gse_out, pin(hit, "ExplicitTime", "in"))
        connect(gse_out, pin(push, "ExplicitTime", "in"))
        blb = info(create(AG, BLB, px, py))
        connect(pin(push, "Pose", "out"), pin(blb, "BlendPose_0", "in"))   # true = 밀침
        connect(pin(hit, "Pose", "out"), pin(blb, "BlendPose_1", "in"))    # false = 타격
        setval(pin(blb, "BlendTime_0", "in"), 0.05)
        setval(pin(blb, "BlendTime_1", "in"), 0.05)
        gps = info(create(AG, VGET % "IsPushStun", px - 300, py + 180))
        connect(pin(gps, "IsPushStun", "out"), pin(blb, "bActiveValue", "in"))
        call("connect_pins", output_pin=pin(blb, "Pose", "out")["pin_id"], input_pin=target)
        log.append("HitReact -> bool blend(PushReact/HitReact)")

    call("compile_blueprint", blueprint=ref(BP))
    execute_tool("editor_toolset.toolsets.asset.AssetTools.save_assets", json.dumps({"asset_paths": [BP.split(".")[0]]}))
    return {"species": SP, "log": log}
