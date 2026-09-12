"""데이터 표 행·WBP의 직접 친 텍스트를 String Table "CL9" 참조로 일괄 변환한다 (docs/systems/localization.md).

에디터 안에서 실행: 콘솔에  py "D:/Work/unreal/CL9/Tools/Unreal/convert_text_to_string_table.py"
- 1단계(dry run, 기본): 바꿀 대상을 로그로만 찍는다. 인자로 apply를 주면(py "...py" apply) 실제로 바꾸고 저장한다.
- 데이터 행: /Game/Data 아래 모든 UCL9DefTable 행과 독립 UCL9DefBase 에셋의 DisplayName/Description을
  `<종류>.<Alias>[.<SubAlias>].Name|Desc` 키 참조로 바꾸고, 없는 키는 현재 한글 원문과 함께 마스터 시트(CL9_Strings.csv) 끝에 붙인다.
  끝나면 `python Tools/Loc/sync_strings.py --build`로 런타임 CSV·번역을 다시 만든다.
- WBP: 아래 WBP_KEYS에 적힌 (WBP 이름, 위젯 이름) → 키로 바꾸고, WBP_INVARIANT의 기호는 문화권 불변으로 만든다.
  적혀 있지 않은 한글 텍스트는 경고로 남긴다.
"""
import csv
import io
import os
import sys

import unreal

APPLY = "apply" in sys.argv[1:]
TABLE = "CL9"
CSV_PATH = os.path.join(unreal.Paths.project_content_dir(), "Strings", "CL9_Strings.csv")  # 마스터 시트 (No,Key,DevNotes,ko,en,...)
WBP_DIR = "/Game/UI"
DATA_DIR = "/Game/Data"

# (WBP 에셋 이름, TextBlock 위젯 이름) → 키. 자리표시자는 C++이 쓰는 키와 같은 것을 고른다.
WBP_KEYS = {
    ("WBP_Dawn", "CloseText"): "Dawn.Close",
    ("WBP_Dawn", "ResultText"): "Dawn.Earned",
    ("WBP_Dawn", "TitleText"): "Dawn.TitlePlain",
    ("WBP_HUD", "HpText"): "Hud.Hp",
    ("WBP_HUD", "ClockText"): "Hud.Lobby",
    ("WBP_HUD", "HeadText"): "Hud.HeadEmpty",
    ("WBP_HUD", "OrdersText"): "Hud.OrderLine",
    ("WBP_HUD", "PeriodText"): "Hud.Period",
    ("WBP_HUD", "BuffText"): "Hud.BuffLine",
    ("WBP_HUD", "NightWarnText"): "Hud.NightWarn",
    ("WBP_HUD", "HandText"): "Hud.HandEmpty",
    ("WBP_HUD", "PocketText"): "Hud.PocketEmpty",
    ("WBP_HUD", "PromptText"): "Prompt.PickUp",
    ("WBP_HUD", "MoneyText"): "Hud.Money",
    ("WBP_HUD", "CounterText"): "Hud.CounterEmpty",
    ("WBP_HUD", "ItemInfoText"): "Hud.ItemWeight",
    ("WBP_HUD", "ItemNameText"): "ScanLabel.Name",
    ("WBP_HUD", "ItemPriceText"): "ScanLabel.Value",
    ("WBP_OrderSelect", "AcceptText"): "OrderSelect.Accept",
    ("WBP_OrderSelect", "HintText"): "OrderSelect.HintHost",
    ("WBP_OrderSelect", "TitleText"): "OrderSelect.Title",
    ("WBP_Pause", "Text_Continue"): "Pause.Continue",
    ("WBP_Pause", "Text_Leave"): "Pause.LeaveLobby",
    ("WBP_Pause", "Text_Quit"): "Pause.Quit",
    ("WBP_Pause", "Text_Settings"): "Pause.Settings",
    ("WBP_Pause", "HintText"): "Pause.NotReady",
    ("WBP_Pause", "TitleText"): "Pause.Title",
    ("WBP_ScanLabel", "NameText"): "ScanLabel.Name",
    ("WBP_ScanLabel", "PriceText"): "ScanLabel.Value",
    ("WBP_Shop", "CloseText"): "Shop.Close",
    ("WBP_Shop", "MoneyText"): "Shop.Money",
    ("WBP_Shop", "TitleText"): "Shop.Title",
    ("WBP_Spectate", "NextButtonText"): "Spectate.Next",
    ("WBP_Spectate", "HintText"): "Spectate.ReturnHint",
    ("WBP_Spectate", "TargetNameText"): "Spectate.Searching",
}

# 언어와 무관한 기호(하트 글리프 등) — 번역 대상이 아니라 문화권 불변 텍스트로 바꿔 수집에서 뺀다.
WBP_INVARIANT = {
    ("WBP_HUD", "HeartGlyph1"),
    ("WBP_HUD", "HeartGlyph2"),
    ("WBP_HUD", "HeartGlyph3"),
}

log = unreal.log
warn = unreal.log_warning


def read_csv():
    rows = []
    with io.open(CSV_PATH, "r", encoding="utf-8-sig", newline="") as f:
        for r in csv.reader(f):
            rows.append(r)
    header, body = rows[0], rows[1:]
    ko = header.index("ko")
    keys = {r[1]: r[ko] for r in body if r}
    return header, body, keys


def append_csv(header, body, new_rows):
    with io.open(CSV_PATH, "w", encoding="utf-8-sig", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(header)
        for r in body:
            w.writerow(r)
        next_no = max([int(r[0]) for r in body if r and r[0].isdigit()] + [0]) + 1
        for key, source, note in new_rows:
            w.writerow([next_no, key, note, source] + [""] * (len(header) - 4))
            next_no += 1


def is_table_ref(text):
    return unreal.TextLibrary.text_is_from_string_table(text)


def def_kind(row):
    name = row.get_class().get_name()  # CL9ItemDef → Item
    if name.startswith("CL9"):
        name = name[3:]
    if name.endswith("Def"):
        name = name[:-3]
    return name


def convert_row(row, keys, new_rows, note):
    changed = False
    alias = str(row.get_editor_property("alias"))
    sub = str(row.get_editor_property("sub_alias"))
    base = "%s.%s" % (def_kind(row), alias)
    if sub and sub != "None":
        base += "." + sub
    for prop, suffix in (("display_name", "Name"), ("description", "Desc")):
        text = row.get_editor_property(prop)
        source = str(text)
        if not source:
            continue
        if is_table_ref(text):
            continue
        key = "%s.%s" % (base, suffix)
        if key in keys and keys[key] != source:
            raise RuntimeError("key %s already has '%s', row has '%s'" % (key, keys[key], source))
        if key not in keys:
            keys[key] = source
            new_rows.append([key, source, note])
        log("[loc] %s.%s: '%s' -> %s" % (note, prop, source, key))
        if APPLY:
            row.set_editor_property(prop, unreal.TextLibrary.text_from_string_table(TABLE, key))
            changed = True
    return changed


def convert_defs(keys, new_rows):
    saved = []
    DEF_TABLE = unreal.load_class(None, "/Script/CL9.CL9DefTable")
    DEF_BASE = unreal.load_class(None, "/Script/CL9.CL9DefBase")
    for path in unreal.EditorAssetLibrary.list_assets(DATA_DIR, True, False):
        asset = unreal.load_asset(path.split(".")[0])
        if asset is None:
            continue
        name = asset.get_name()
        changed = False
        if unreal.MathLibrary.class_is_child_of(asset.get_class(), DEF_TABLE):
            for row in asset.get_editor_property("rows"):
                if row is None:
                    continue
                changed |= convert_row(row, keys, new_rows, name)
        elif unreal.MathLibrary.class_is_child_of(asset.get_class(), DEF_BASE):
            changed |= convert_row(asset, keys, new_rows, name)
        else:
            continue
        if changed and APPLY:
            asset.modify()
            if unreal.EditorAssetLibrary.save_loaded_asset(asset, False):
                saved.append(name)
            else:
                warn("[loc] save failed: %s" % name)
    return saved


def has_hangul(s):
    return any("\uac00" <= ch <= "\ud7a3" for ch in s)


def convert_widgets(keys):
    saved = []
    for path in unreal.EditorAssetLibrary.list_assets(WBP_DIR, False, False):
        if "/WBP_" not in path:
            continue
        wb = unreal.load_asset(path.split(".")[0])
        if wb is None:
            continue
        wb_name = wb.get_name()
        prefix = wb.get_path_name() + ":WidgetTree."
        changed = False
        for tb in unreal.ObjectIterator(unreal.TextBlock):
            full = tb.get_path_name()
            if not full.startswith(prefix):
                continue
            text = tb.get_editor_property("text")
            source = str(text)
            if is_table_ref(text) or not source.strip() or unreal.TextLibrary.text_is_culture_invariant(text):
                continue
            widget = tb.get_name()
            if (wb_name, widget) in WBP_INVARIANT:
                log("[loc] %s.%s: '%s' -> culture invariant" % (wb_name, widget, source))
                if APPLY:
                    tb.set_editor_property("text", unreal.TextLibrary.conv_string_to_text(source))
                    changed = True
                continue
            key = WBP_KEYS.get((wb_name, widget))
            if key is None:
                if has_hangul(source):
                    warn("[loc] %s.%s has typed text '%s' and no key in WBP_KEYS" % (wb_name, widget, source))
                else:
                    log("[loc] %s.%s: '%s' (no mapping, left as is)" % (wb_name, widget, source))
                continue
            if key not in keys:
                warn("[loc] %s.%s -> %s is not in the CSV" % (wb_name, widget, key))
            log("[loc] %s.%s: '%s' -> %s" % (wb_name, widget, source, key))
            if APPLY:
                tb.set_editor_property("text", unreal.TextLibrary.text_from_string_table(TABLE, key))
                changed = True
        if changed and APPLY:
            wb.modify()
            unreal.BlueprintEditorLibrary.compile_blueprint(wb)
            if unreal.EditorAssetLibrary.save_loaded_asset(wb, False):
                saved.append(wb_name)
            else:
                warn("[loc] save failed: %s" % wb_name)
    return saved


def main():
    header, body, keys = read_csv()
    new_rows = []
    saved = convert_defs(keys, new_rows)
    saved += convert_widgets(keys)
    if APPLY and new_rows:
        append_csv(header, body, new_rows)
        log("[loc] appended %d rows to %s" % (len(new_rows), CSV_PATH))
    log("[loc] %s — new keys %d, saved assets: %s" % ("APPLIED" if APPLY else "DRY RUN", len(new_rows), ", ".join(saved) or "-"))


main()
