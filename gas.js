// ===== あなたのシート名に合わせて変更 =====
const COUNT_SHEET_NAME = "count"; // カウントを置いているシート
const STATE_SHEET_NAME = "state";   // 差分用（なければ作ってください）
// counts のセル（あなたの形式に合わせた：B1,B2,B3）
const CELL_A = "B1";
const CELL_B = "B2";
const CELL_C = "B3";
// state のセル（last値）
const LAST_A = "A2";
const LAST_B = "B2";
const LAST_C = "C2";
function sh_(name) {
  const ss = SpreadsheetApp.getActiveSpreadsheet();
  const sh = ss.getSheetByName(name);
  if (!sh) throw new Error("Sheet not found: " + name);
  return sh;
}
function num_(v) {
  const n = Number(v);
  return Number.isFinite(n) ? n : 0;
}
function json_(obj) {
  return ContentService.createTextOutput(JSON.stringify(obj))
    .setMimeType(ContentService.MimeType.JSON);
}
// 受信機起動時など：合計もlastもゼロにする
function resetAll_() {
  const countSh = sh_(COUNT_SHEET_NAME);
  countSh.getRange(CELL_A).setValue(0);
  countSh.getRange(CELL_B).setValue(0);
  countSh.getRange(CELL_C).setValue(0);
  const stateSh = sh_(STATE_SHEET_NAME);
  stateSh.getRange("A1").setValue("lastA");
  stateSh.getRange("B1").setValue("lastB");
  stateSh.getRange("C1").setValue("lastC");
  stateSh.getRange(LAST_A).setValue(0);
  stateSh.getRange(LAST_B).setValue(0);
  stateSh.getRange(LAST_C).setValue(0);
  SpreadsheetApp.flush();
}
// 送信用から来た増分を合計に加算
function addCount_(btn, count) {
  const countSh = sh_(COUNT_SHEET_NAME);
  count = Math.max(0, Math.floor(num_(count)));
  if (btn === "A") countSh.getRange(CELL_A).setValue(num_(countSh.getRange(CELL_A).getValue()) + count);
  if (btn === "B") countSh.getRange(CELL_B).setValue(num_(countSh.getRange(CELL_B).getValue()) + count);
  if (btn === "C") countSh.getRange(CELL_C).setValue(num_(countSh.getRange(CELL_C).getValue()) + count);
  SpreadsheetApp.flush();
}
// 受信機が取りに来たら「前回からの差分」を返す
function getDiff_() {
  const countSh = sh_(COUNT_SHEET_NAME);
  const stateSh = sh_(STATE_SHEET_NAME);
  const A = num_(countSh.getRange(CELL_A).getValue());
  const B = num_(countSh.getRange(CELL_B).getValue());
  const C = num_(countSh.getRange(CELL_C).getValue());
  const lastA = num_(stateSh.getRange(LAST_A).getValue());
  const lastB = num_(stateSh.getRange(LAST_B).getValue());
  const lastC = num_(stateSh.getRange(LAST_C).getValue());
  // 差分（万一リセット等で小さくなった場合は0扱い）
  const dA = Math.max(0, A - lastA);
  const dB = Math.max(0, B - lastB);
  const dC = Math.max(0, C - lastC);
  // 次回のために last を更新
  stateSh.getRange(LAST_A).setValue(A);
  stateSh.getRange(LAST_B).setValue(B);
  stateSh.getRange(LAST_C).setValue(C);
  SpreadsheetApp.flush();
  return { ok: true, A: dA, B: dB, C: dC, totalA: A, totalB: B, totalC: C };
}
// ===== Webアプリ入口 =====
function doGet(e) {
  const action = (e && e.parameter && e.parameter.action) ? String(e.parameter.action) : "diff";
  if (action === "diff") {
    return json_(getDiff_());
  }
  // 動作確認用：合計を返す
  if (action === "totals") {
    const countSh = sh_(COUNT_SHEET_NAME);
    const A = num_(countSh.getRange(CELL_A).getValue());
    const B = num_(countSh.getRange(CELL_B).getValue());
    const C = num_(countSh.getRange(CELL_C).getValue());
    return json_({ ok: true, A, B, C });
  }
  return json_({ ok: false, error: "unknown action" });
}
function doPost(e) {
  const action = (e && e.parameter && e.parameter.action) ? String(e.parameter.action) : "";
  // 受信機起動時リセット
  if (action === "reset") {
    resetAll_();
    return ContentService.createTextOutput("RESET_OK");
  }
  // 送信用：btn/count を受け取って加算
  const btn = (e && e.parameter && e.parameter.btn) ? String(e.parameter.btn) : "";
  const count = (e && e.parameter && e.parameter.count) ? e.parameter.count : 0;
  if (!["A", "B", "C"].includes(btn)) {
    return ContentService.createTextOutput("ERROR:INVALID_BTN");
  }
  addCount_(btn, count);
  return ContentService.createTextOutput("OK");
}
