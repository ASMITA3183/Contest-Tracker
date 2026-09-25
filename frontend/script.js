async function loadContests() {
  const tableBody = document.getElementById("contest-table-body");

  try {
    const response = await fetch("/contests");

    const contests = await response.json();

    contests.forEach((contest, index) => {
      const row = document.createElement("tr");

      // dateTime is a Unix timestamp in seconds, shown in the browser's time zone
      const date = new Date(contest.dateTime * 1000).toLocaleString("en-IN", {
        dateStyle: "medium",
        timeStyle: "short",
      });

      row.innerHTML = `
            <td>${contests.length - index}</td>
            <td>${date}</td>
            <td>${contest.platform}</td>
            <td>${contest.contestName}</td>
            <td>${contest.solved}/${contest.totalQuestions}</td>
            <td>${contest.rank}</td>
            <td>${contest.rating}</td>
        `;

      tableBody.appendChild(row);
    });
  } catch (error) {
    tableBody.innerHTML = `<tr><td colspan="6">Could not load contests: ${error}</td></tr>`;
  }
}

loadContests();
