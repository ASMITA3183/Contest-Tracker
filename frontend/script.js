const form = document.getElementById("username-form");
const submitButton = document.getElementById("submit-button");
const statusText = document.getElementById("status");
const tableBody = document.getElementById("contest-table-body");

function addCell(row, text) {
  const cell = document.createElement("td");
  cell.textContent = text;
  row.appendChild(cell);
}

function showContests(contests) {
  tableBody.innerHTML = "";

  contests.forEach((contest, index) => {
    const row = document.createElement("tr");

    // dateTime is a Unix timestamp in seconds, shown in the browser's time zone
    const date = new Date(contest.dateTime * 1000).toLocaleString("en-IN", {
      dateStyle: "medium",
      timeStyle: "short",
    });

    addCell(row, contests.length - index);
    addCell(row, date);
    addCell(row, contest.platform);
    addCell(row, contest.contestName);
    addCell(row, `${contest.solved}/${contest.totalQuestions}`);
    addCell(row, contest.rank);
    addCell(row, contest.rating);

    tableBody.appendChild(row);
  });
}

form.addEventListener("submit", async (event) => {
  event.preventDefault();

  const usernames = {
    leetcode: document.getElementById("leetcode").value.trim(),
    codechef: document.getElementById("codechef").value.trim(),
    codeforces: document.getElementById("codeforces").value.trim(),
  };

  if (!usernames.leetcode && !usernames.codechef && !usernames.codeforces) {
    alert("Enter at least one username");
    return;
  }

  submitButton.disabled = true;
  statusText.textContent = "Fetching contests...";
  tableBody.innerHTML = "";

  try {
    const response = await fetch("/contests?" + new URLSearchParams(usernames));

    const result = await response.json();

    showContests(result.contests);

    statusText.textContent = `${result.contests.length} contests found`;

    // One popup listing every platform that failed, e.g. "CodeChef: User not found"
    if (result.errors.length > 0) {
      alert(
        result.errors
          .map((error) => `${error.platform}: ${error.message}`)
          .join("\n"),
      );
    }
  } catch (error) {
    statusText.textContent = "";
    alert(`Could not load contests: ${error}`);
  } finally {
    submitButton.disabled = false;
  }
});
