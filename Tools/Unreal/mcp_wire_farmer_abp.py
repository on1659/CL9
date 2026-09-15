"""ABP_Unarmed(전신, __VIEW__=tp) / ABP_FP_View(1인칭, fp) 넘어짐·던지기 가지 배선(2026-09-15) — 에디터 MCP
`ProgrammaticToolset.execute_tool_script` 에 이 파일 내용을 넘긴다(__VIEW__ 치환). 변수 KnockdownElapsed·KnockdownRemaining·ThrowElapsed·
ThrowWindupElapsed(+tp IsDead) 추가, EventGraph 마지막 Set 뒤에 CL9 게터 세터 체인, AnimGraph 루트 앞에 넘어짐 부울 블렌드(쓰러짐 평가기 →
일어남 평가기 → 누움(전신: 평가기 fmod/IsDead 정지, 1인칭: FPArmsDown)), 던지기 가지(전신: Hold 레이어 머리 + 가중치 OR, 1인칭: 루트 아래).
한 번만 돌린다(두 번 돌리면 가지가 겹친다). docs/systems/animation.md "넘어짐 가지"·"던지기 가지".
"""
import json
VIEW = "__VIEW__"    # "tp" = ABP_Unarmed(전신), "fp" = ABP_FP_View(1인칭 팔)
BT = "editor_toolset.toolsets.blueprint.BlueprintTools."
BP = "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed" if VIEW == "tp" else "/Game/Characters/FarmerA/Anims/ABP_FP_View.ABP_FP_View"
AG = BP + ":AnimGraph"
EG = BP + ":EventGraph"
EVAL = "애니메이션|시퀀스|'%s’평가"
PLAY = "애니메이션|시퀀스|'%s'재생"
BLB = "Animation|Blends|부울로포즈블렌딩"
VGET = "Variables|디폴트|Get%s"
VSET = "Variables|디폴트|Set%s"
INRANGE = "수학|플로트|InRange(Float)"
MAPR = "수학|플로트|MapRangeUnclamped"
FMOD = "수학|플로트|%(Float)"
SELF = "수학|플로트|SelectFloat"
OR = "수학|부울|ORBoolean"
P = "AN_Farmer_" + ("" if VIEW == "tp" else "FP")


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
    return info(call("create_node", graph=ref(graph), type_id=type_id, pos={"x": int(x), "y": int(y)})["returnValue"])


def connect(out_pin, in_pin):
    call("connect_pins", output_pin=out_pin["pin_id"], input_pin=in_pin["pin_id"])


def connect_id(out_pin, in_pin_id):
    call("connect_pins", output_pin=out_pin["pin_id"], input_pin=in_pin_id)


def brk(out_pin, in_pin_id):
    call("break_pins", output_pin=out_pin["pin_id"], input_pin=in_pin_id)


def setval(p, v):
    call("set_pin_value", pin=p["pin_id"], value=str(v))


def inrange(graph, value_pin, lo, hi, x, y):
    n = create(graph, INRANGE, x, y)
    connect(value_pin, pin(n, "Value", "in"))
    setval(pin(n, "Min", "in"), lo)
    setval(pin(n, "Max", "in"), hi)
    return n


def blb(graph, x, y, true_pose, false_pose, active, bt=0.1):
    n = create(graph, BLB, x, y)
    connect(true_pose, pin(n, "BlendPose_0", "in"))
    if false_pose is not None:
        connect(false_pose, pin(n, "BlendPose_1", "in"))
    connect(active, pin(n, "bActiveValue", "in"))
    setval(pin(n, "BlendTime_0", "in"), bt)
    setval(pin(n, "BlendTime_1", "in"), bt)
    return n


def run():
    log = []
    names = ["KnockdownElapsed", "KnockdownRemaining", "ThrowElapsed", "ThrowWindupElapsed"] + (["IsDead"] if VIEW == "tp" else [])
    vars_ = call("list_variables", blueprint=ref(BP))["returnValue"]
    for v in names:
        if v not in vars_:
            call("add_variable", blueprint=ref(BP), name=v, type_name="bool" if v == "IsDead" else "float")
    log.append("vars ok")

    # ---- EventGraph: 마지막 Set 뒤에 게터 → Set 체인 ----
    eg = infos(find_all(EG))
    if not any(n["type_id"].endswith("|SetKnockdownElapsed") for n in eg):
        cast = next(n for n in eg if "CastToCL9Character" in n["type_id"])
        as_char = pin(cast, "AsCL9Character", "out")
        tail_name = "|SetAimPitch" if VIEW == "tp" else "|SetGroundSpeed"
        tail = next(n for n in eg if n["type_id"].endswith(tail_name))
        prev_then = pin(tail, "then", "out")
        x, y = tail["position"]["x"] + 260, tail["position"]["y"]
        getters = {"KnockdownElapsed": "CL9|GetKnockdownElapsed", "KnockdownRemaining": "CL9|GetKnockdownRemaining",
                   "ThrowElapsed": "CL9|GetThrowElapsed", "ThrowWindupElapsed": "CL9|GetThrowWindupElapsed", "IsDead": "CL9|IsDead"}
        for v in names:
            f = create(EG, getters[v], x - 60, y + 200)
            connect(as_char, pin(f, "self", "in"))
            s = create(EG, VSET % v, x, y)
            connect(prev_then, pin(s, "execute", "in"))
            connect(pin(f, "ReturnValue", "out"), pin(s, v, "in"))
            prev_then = pin(s, "then", "out")
            x += 260
        log.append("eventgraph wired")

    # ---- AnimGraph ----
    ag = infos(find_all(AG))
    root = next(n for n in ag if n["type_id"].endswith("|출력포즈"))
    root_in = pin(root, "Result", "in")
    final_src = root_in["connected_pins"][0]      # tp: ControlRig.Pose / fp: BlendListByBool_2.Pose
    final_node = info(final_src["node"])
    final_pose = pin(final_node, "Pose", "out")
    rx, ry = root["position"]["x"], root["position"]["y"]
    brk(final_pose, root_in["pin_id"])

    g_ke = create(AG, VGET % "KnockdownElapsed", rx - 1500, ry + 900)
    g_kr = create(AG, VGET % "KnockdownRemaining", rx - 1500, ry + 1100)
    g_te = create(AG, VGET % "ThrowElapsed", rx - 1500, ry + 1400)
    g_tw = create(AG, VGET % "ThrowWindupElapsed", rx - 1500, ry + 1600)
    ke = pin(g_ke, "KnockdownElapsed", "out")
    kr = pin(g_kr, "KnockdownRemaining", "out")
    te = pin(g_te, "ThrowElapsed", "out")
    tw = pin(g_tw, "ThrowWindupElapsed", "out")

    # 넘어짐 가지: 쓰러짐 평가기 / 일어남 평가기 / 누움
    ev_kd = create(AG, EVAL % (P + "Knockdown"), rx - 1100, ry + 800)
    connect(ke, pin(ev_kd, "ExplicitTime", "in"))
    ev_gu = create(AG, EVAL % (P + "GetUp"), rx - 1100, ry + 1000)
    m_gu = create(AG, MAPR, rx - 1300, ry + 1000)          # 0.6 − Remaining
    connect(kr, pin(m_gu, "Value", "in"))
    setval(pin(m_gu, "InRangeA", "in"), 0.0); setval(pin(m_gu, "InRangeB", "in"), 1.0)
    setval(pin(m_gu, "OutRangeA", "in"), 0.6); setval(pin(m_gu, "OutRangeB", "in"), -0.4)
    connect(pin(m_gu, "ReturnValue", "out"), pin(ev_gu, "ExplicitTime", "in"))
    if VIEW == "tp":
        # 누움 = 평가기, ExplicitTime = 죽었으면 1.0 고정, 아니면 (Elapsed−0.5) mod 2
        m_dn = create(AG, MAPR, rx - 1300, ry + 1200)
        connect(ke, pin(m_dn, "Value", "in"))
        setval(pin(m_dn, "InRangeA", "in"), 0.0); setval(pin(m_dn, "InRangeB", "in"), 1.0)
        setval(pin(m_dn, "OutRangeA", "in"), -0.5); setval(pin(m_dn, "OutRangeB", "in"), 0.5)
        fm = create(AG, FMOD, rx - 1150, ry + 1200)
        connect(pin(m_dn, "ReturnValue", "out"), pin(fm, "A", "in"))
        setval(pin(fm, "B", "in"), 2.0)
        g_dead = create(AG, VGET % "IsDead", rx - 1300, ry + 1320)
        sel = create(AG, SELF, rx - 1000, ry + 1220)
        setval(pin(sel, "A", "in"), 1.0)
        connect(pin(fm, "ReturnValue", "out"), pin(sel, "B", "in"))
        connect(pin(g_dead, "IsDead", "out"), pin(sel, "bPickA", "in"))
        down = create(AG, EVAL % (P + "Down"), rx - 850, ry + 1200)
        connect(pin(sel, "ReturnValue", "out"), pin(down, "ExplicitTime", "in"))
    else:
        down = create(AG, PLAY % (P + "ArmsDown"), rx - 850, ry + 1200)
    ir_gu = inrange(AG, kr, 0.0, 0.6, rx - 900, ry + 1100)
    b_gu = blb(AG, rx - 650, ry + 1000, pin(ev_gu, "Pose", "out"), pin(down, "Pose", "out"), pin(ir_gu, "ReturnValue", "out"))
    ir_kd = inrange(AG, ke, 0.0, 0.5, rx - 900, ry + 700)
    b_kp = blb(AG, rx - 450, ry + 800, pin(ev_kd, "Pose", "out"), pin(b_gu, "Pose", "out"), pin(ir_kd, "ReturnValue", "out"), 0.05)
    ir_k = inrange(AG, ke, 0.0, 100000.0, rx - 900, ry + 550)

    # 던지기 가지 (1인칭은 루트 바로 아래, 전신은 Hold 체인 머리)
    ev_th = create(AG, EVAL % (P + "Throw"), rx - 1100, ry + 1450)
    connect(te, pin(ev_th, "ExplicitTime", "in"))
    pl_tw = create(AG, PLAY % (P + "ThrowWindup"), rx - 1100, ry + 1650)
    ir_th = inrange(AG, te, 0.0, 0.333, rx - 900, ry + 1500)
    ir_tw = inrange(AG, tw, 0.0, 100000.0, rx - 900, ry + 1700)

    if VIEW == "fp":
        b_tw = blb(AG, rx - 650, ry + 1600, pin(pl_tw, "Pose", "out"), final_pose, pin(ir_tw, "ReturnValue", "out"))
        b_th = blb(AG, rx - 450, ry + 1450, pin(ev_th, "Pose", "out"), pin(b_tw, "Pose", "out"), pin(ir_th, "ReturnValue", "out"), 0.05)
        b_k = blb(AG, rx - 250, ry + 300, pin(b_kp, "Pose", "out"), pin(b_th, "Pose", "out"), pin(ir_k, "ReturnValue", "out"))
    else:
        # 전신: 던지기는 상체 레이어(LayeredBoneBlend.BlendPoses_0)의 머리에 끼우고, 레이어 가중치에 OR 로 더한다
        layer = next(n for n in ag if "본별로레이어드블렌딩" in n["type_id"])
        head_in = pin(layer, "BlendPoses_0", "in")
        head_src = head_in["connected_pins"][0]                       # BlendListByBool_5.Pose
        head_node = info(head_src["node"])
        head_pose = pin(head_node, "Pose", "out")
        brk(head_pose, head_in["pin_id"])
        b_tw = blb(AG, rx - 650, ry + 1600, pin(pl_tw, "Pose", "out"), head_pose, pin(ir_tw, "ReturnValue", "out"))
        b_th = blb(AG, rx - 450, ry + 1450, pin(ev_th, "Pose", "out"), pin(b_tw, "Pose", "out"), pin(ir_th, "ReturnValue", "out"), 0.05)
        connect_id(pin(b_th, "Pose", "out"), head_in["pin_id"])
        w_in = pin(layer, "BlendWeights_0", "in")
        tofloat = info(w_in["connected_pins"][0]["node"])             # ToFloat(Boolean)
        tf_in = pin(tofloat, "InBool", "in")
        old_bool_src = tf_in["connected_pins"][0]                     # NOT(HoldPose==None).ReturnValue
        old_bool_node = info(old_bool_src["node"])
        brk(pin(old_bool_node, "ReturnValue", "out"), tf_in["pin_id"])
        or2 = create(AG, OR, rx - 1150, ry + 1850)
        connect(pin(ir_th, "ReturnValue", "out"), pin(or2, "A", "in"))
        connect(pin(ir_tw, "ReturnValue", "out"), pin(or2, "B", "in"))
        or1 = create(AG, OR, rx - 950, ry + 1850)
        connect(pin(old_bool_node, "ReturnValue", "out"), pin(or1, "A", "in"))
        connect(pin(or2, "ReturnValue", "out"), pin(or1, "B", "in"))
        connect(pin(or1, "ReturnValue", "out"), tf_in)
        b_k = blb(AG, rx - 250, ry + 300, pin(b_kp, "Pose", "out"), final_pose, pin(ir_k, "ReturnValue", "out"))
    connect_id(pin(b_k, "Pose", "out"), root_in["pin_id"])
    log.append("animgraph wired")

    call("compile_blueprint", blueprint=ref(BP))
    execute_tool("editor_toolset.toolsets.asset.AssetTools.save_assets", json.dumps({"asset_paths": [BP.split(".")[0]]}))
    return {"view": VIEW, "log": log}
