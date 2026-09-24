console.log("SCRIPT STARTED");

async function loadContests() {
  const response = await fetch("http://localhost:8080/contests");

  const contests = await response.json();

  const tableBody = document.getElementById("contest-table-body");

  contests.forEach((contest, index) => {
    const row = document.createElement("tr");

    row.innerHTML = `
            <td>${contests.length - index}</td>
            <td>${contest.dateTime}</td>
            <td>${contest.platform}</td>
            <td>${contest.contestName}</td>
            <td>${contest.solved}/${contest.totalQuestions}</td>
            <td>${contest.rank}</td>
        `;

    tableBody.appendChild(row);
  });
}

loadContests();
